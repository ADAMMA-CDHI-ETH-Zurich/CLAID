/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Stephan Altmüller
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*         http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
***************************************************************************/

#include "dispatch/core/middleware.hh"
#include "dispatch/core/Configuration/Configuration.hh"
#include "dispatch/core/CLAID.hh"
#include "dispatch/core/Logger/Logger.hh"
#include "dispatch/core/Router/RoutingTreeParser.hh"
#include "dispatch/core/Utilities/Path.hh"
#include "dispatch/core/Utilities/FileUtils.hh"
#include "absl/strings/str_split.h"
#include <stdexcept>

using namespace claid;
using namespace std;

MiddleWare::MiddleWare(const string& socketPath, const string& configurationPath,
    const string& hostId, const string& userId, const string& deviceId)
        : socketPath(socketPath), configurationPath(configurationPath),
          hostId(hostId), userId(userId), deviceId(deviceId), remoteFunctionRunnableHandler("MIDDLEWARE", moduleTable.inputQueue())
    {
        moduleTable.setProperties(ModuleTableProperties{userId, deviceId});
        this->logMessagesQueue = std::make_shared<SharedQueue<LogMessage>>();
        this->eventTracker = std::make_shared<EventTracker>();

        remoteFunctionRunnableHandler
            .registerRunnable(
                "get_all_running_modules_of_all_runtimes", &MiddleWare::getAllRunningModulesOfAllRuntimes, this);
    
        remoteFunctionRunnableHandler
            .registerRunnable(
                "add_loose_direct_subscription", &MiddleWare::addLooseDirectSubscription, this);

        remoteFunctionRunnableHandler
            .registerRunnable(
                "add_loose_direct_subscription_if_not_exists", &MiddleWare::addLooseDirectSubscriptionIfNotExists, this);

        remoteFunctionRunnableHandler
            .registerRunnable(
                "remove_loose_direct_subscription", &MiddleWare::removeLooseDirectSubscription, this);
    }

MiddleWare::~MiddleWare() {

    Logger::logInfo("Middleware destructor");
    if(!running)
    {
        return;
    }

    auto status = shutdown();
    if (!status.ok()) {
        // TODO: replace with proper logging
        Logger::logInfo("Error running shutdown in MiddleWare destructor: %s", string(status.message()).c_str());
    }
}

absl::Status MiddleWare::start() {

    if(running)
    {
        return absl::AbortedError("Failed to start middleware, middleware is already running. Middleware::start was called twice");
    }

    // Fill the module table fromt he config file
    Configuration config;
    Logger::logInfo("Parsing %s\n", configurationPath.c_str());
    auto status = config.parseFromJSONFile(configurationPath);
    if (!status.ok()) {
        Logger::logInfo("Unable to parse config file: %s", string(status.message()).c_str());
        return status;
    }
    Logger::setMinimumSeverityLevelToPrint(config.getMinLogSeverityLevelToPrint(this->hostId));

    HostDescriptionMap hostDescriptions;

    // All Modules specified in the configuration file.
    // Needed by the Router.
    ModuleDescriptionMap allModuleDescriptions;

    // Subset of allModuleDescriptions. Contains only the Modules running on the current host.
    // Needed to populate the ModuleTable.
    ModuleDescriptionMap hostModuleDescriptions;

    ChannelDescriptionMap channelDescriptions;

    status = getHostModuleAndChannelDescriptions(hostId, config, hostDescriptions, allModuleDescriptions, hostModuleDescriptions, channelDescriptions);
    if (!status.ok()) {
        return status;
    }


    status = populateModuleTable(allModuleDescriptions, hostModuleDescriptions, channelDescriptions, moduleTable);
    if (!status.ok()) {
        return status;
    }
    Logger::logInfo("Module Table:");
    Logger::logInfo("%s", moduleTable.toString().c_str());

    localDispatcher = make_unique<DispatcherServer>(socketPath, moduleTable);
    if (!localDispatcher->start()) {
        return absl::InternalError("Error starting local dispatcher !");
    }

    status = this->startRouter(this->hostId, hostDescriptions, allModuleDescriptions);
    if (!status.ok()) {
        return status;
    }

    status = this->startRemoteDispatcherServer(this->hostId, hostDescriptions);
    if(!status.ok()) {
        return status;
    }

    status = this->startRemoteDispatcherClient(this->hostId, this->userId, this->deviceId, hostDescriptions);
    if(!status.ok())
    {
        return status;
    }

    // if(config.isDesignerModeEnabled())
    // {
    //     this->enableDesignerMode();
    // }
    // else
    // {
    //     this->disableDesignerMode();
    // }

    Logger::logInfo("Middleware started.");

    running = true;

    this->controlPackageHandlerThread = std::make_unique<std::thread>([this]() {
        this->readControlPackages();
    });

    this->logMessageHandlerThread = std::make_unique<std::thread>([this]() {
        this->readLocalLogMessages();
    });

    this->currentConfiguration = config;
    this->setupLogSink();

    this->eventTracker->onCLAIDStarted();

    if(config.needToCheckIfAllModulesLoaded())
    {
        std::thread([=]() {
            do {
                // Sleeps at least for the specified deadline.
                // Continues to sleep if any runtime is currently still in initialization (i.e.,
                // Runtime has called the getModuleList() function but not yet completed the InitRuntime() function).)
                std::this_thread::sleep_for(std::chrono::milliseconds(config.getDeadlineForLoadingModulesInMs()));
            }
            while(moduleTable.isAnyRuntimeStillInitializing());

            assertAllModulesLoaded();
        }).detach(); 
    }
    
    return absl::OkStatus();
}

absl::Status MiddleWare::startRemoteDispatcherServer(const std::string& currentHost, const HostDescriptionMap& hostDescriptions)
{
    if(this->remoteDispatcherServer != nullptr)
    {
        return absl::AlreadyExistsError("Failed to start server; RemoteDispatcherServer already exists.");
    }

    // Lookup address of currentHost.
    auto it = hostDescriptions.find(currentHost);
    if(it == hostDescriptions.end())
    {
        return absl::NotFoundError(absl::StrCat(
            "Failed to lookup server address for host \"", currentHost, "\".",
            "The host was not found in the configuration file."
        ));
    }

    const HostDescription& hostDescription = it->second;
    if(!hostDescription.isServer)
    {
        if(hostDescription.hostServerAddress != "")
        {
            return absl::InvalidArgumentError(absl::StrCat(
                "Cannot start RemoteDispatcherServer on host \"", currentHost, "\".\n",
                "The host_server_address is non-empty, but the host was not configured as server (isServer = false).\n"
                "Either set host_server_address to empty (\"\"), or set isServer = true."
            ));
        }
        else
        {
            // If we are not configured as a Server, we will not start the RemoteDispatcherServer.
            return absl::OkStatus();
        }
    }

    // Find out our port from the host description (if we are a server, then hostDescription.hostServerAddress typically would be a port).
    const std::string address = hostDescription.hostServerAddress;

    if(hostDescription.hostServerAddress == "")
    {
        return absl::InvalidArgumentError(absl::StrCat(
            "Cannot start RemoteDispatcherServer on host \"", currentHost, "\".\n",
            "Host was configured as Server (isServer = true), but the host_server_address is empty (\"\").\n"
            "Either specify the host_server_address (\"\"), or set isServer = false."
        ));
    }

    Logger::logInfo("Starting RemoteDispatcherServer, listening on address %s", address.c_str());
    this->remoteDispatcherServer = make_unique<RemoteDispatcherServer>(address, this->hostUserTable);

    return this->remoteDispatcherServer->start();
}

absl::Status MiddleWare::startRemoteDispatcherClient(const std::string& currentHost, const std::string& currentUser,
                const std::string& currentDeviceId, const HostDescriptionMap& hostDescriptions)
{
    if(this->remoteDispatcherClient != nullptr)
    {
        return absl::AlreadyExistsError(absl::StrCat(
            "Failed to start RemoteDispatcherClient on the current host \"", currentHost, "\"; RemoteDispatcherClient already exists."
        ));
    }

    auto it = hostDescriptions.find(currentHost);
    if(it == hostDescriptions.end())
    {
        return absl::NotFoundError(absl::StrCat(
            "Couldn't find host current host \"", currentHost, "\" in host descriptions"
        ));
    }

    const HostDescription& hostDescription = it->second;
    const std::string& targetServer = hostDescription.connectTo;

    if(targetServer == "")
    {
        Logger::logInfo("Not starting RemoteDispatcherClient, because targetServer is empty (this host will not connect to an external host).");
        return absl::OkStatus();
    }

    auto it2 = hostDescriptions.find(targetServer);
    if(it2 == hostDescriptions.end())
    {
        return absl::NotFoundError(absl::StrCat(
            "Current host \"", currentHost, "\" was configured to connect to server \"", targetServer, "\", ",
            "but the server was not found in the host description, i.e., it does not exist in the configuration file."
        ));
    }

    const HostDescription& serverDescription = it2->second;

    if(!serverDescription.isServer)
    {
        return absl::InvalidArgumentError(absl::StrCat(
            "Current host \"", currentHost, "\" was configured to connect to server \"", targetServer, "\", ",
            "but the server was not configured as server in the configuration file (is_server is false)"
        ));
    }

    const std::string& address = serverDescription.hostServerAddress;
    if(address == "")
    {
        return absl::InvalidArgumentError(absl::StrCat(
            "Current host \"", currentHost, "\" was configured to connect to server \"", targetServer, "\", ",
            "but the hostServerAddress of \"", targetServer, "\" is empty and therefore invalid."
        ));
    }

    Logger::logInfo("Starting RemoteDispatcherClient, connecting to address %s", address.c_str());
    this->remoteDispatcherClient = std::make_unique<RemoteDispatcherClient>(address, currentHost, currentUser, currentDeviceId, this->clientTable);
    absl::Status status = this->remoteDispatcherClient->start();

    if(!status.ok())
    {
        return status;
    }

    return absl::OkStatus();
}

absl::Status MiddleWare::startRouter(const std::string& currentHost, const HostDescriptionMap& hostDescriptions, const ModuleDescriptionMap& moduleDescriptions)
{
    Logger::logInfo("Starting router");
    if(this->routingQueueMerger != nullptr)
    {
        return absl::AlreadyExistsError("Failed to start router: RoutingQueueMerger already exists.");
    }

    this->routingQueueMerger = makeUniqueRoutingQueueMerger(this->masterInputQueue, this->moduleTable.inputQueue(), *this->hostUserTable.inputQueue(), this->clientTable.getFromRemoteClientQueue());

    absl::Status status = this->routingQueueMerger->start();
    if(!status.ok())
    {
        return status;
    }

    RoutingTreeParser routingTreeParser;
    status = routingTreeParser.buildRoutingTree(hostDescriptions, routingTree);
    if(!status.ok())
    {
        return status;
    }
    
    std::shared_ptr<LocalRouter> localRouter = std::make_shared<LocalRouter>(currentHost, this->moduleTable);
    std::shared_ptr<ServerRouter> serverRouter = std::make_shared<ServerRouter>(currentHost, routingTree, this->hostUserTable);
    std::shared_ptr<ClientRouter> clientRouter = std::make_shared<ClientRouter>(currentHost, routingTree, this->clientTable);

    for(auto entry : moduleDescriptions)
    {
        Logger::logInfo("%s %s", entry.second.id.c_str(), entry.second.host.c_str());
    }

    this->masterRouter = std::make_unique<MasterRouter>(currentHost, hostDescriptions, moduleDescriptions, this->masterInputQueue, localRouter, clientRouter, serverRouter);

    status = masterRouter->start();
    if(!status.ok())
    {
        return status;
    }


    return absl::OkStatus();
}

void MiddleWare::setupLogSink()
{
    std::unique_lock<std::mutex>(this->logSinkConfigurationMutex);
    Logger::setMinimumSeverityLevelToPrint(this->currentConfiguration.getMinLogSeverityLevelToPrint(this->hostId));
    currentConfiguration.getLogSinkConfiguration(this->logSinkConfiguration, this->logMessagesQueue);
    Logger::setLogSinkConfiguration(this->logSinkConfiguration);
}

void MiddleWare::assertAllModulesLoaded()
{
    std::vector<std::string> notLoadedModules;
    moduleTable.getNotLoadedModules(notLoadedModules);

    if(notLoadedModules.size() > 0)
    {
        std::string errorMessage = absl::StrCat(
             "CLAID Middleware: Terminating as we have missing Modules.\n",
             "The following Modules have not yet been loaded after a deadline of ", currentConfiguration.getDeadlineForLoadingModulesInMs(), " milliseconds\n"
        );

        for(const std::string& module : notLoadedModules)
        {
            std::string moduleType;
            moduleTable.getTypeOfModuleWithId(module, moduleType);
            errorMessage += absl::StrCat("Id: \"", module, "\"\tclass: \"", moduleType, "\"");
        }

        throw std::runtime_error(errorMessage);
    }
}

absl::Status MiddleWare::shutdown()
{
    if(!running)
    {
        return absl::InvalidArgumentError("MiddleWare::shutdown called although middleware was not running.");
    }
    // TODO: remove once the router is added.
    Logger::logInfo("Middleware shutdown called.");

    absl::Status status;
    Logger::logInfo("Stopping MasterRouter");
    
    status = this->masterRouter->stop();
    if(!status.ok())
    {
        return status;
    }

    moduleTable.inputQueue().interruptOnce();
    this->hostUserTable.inputQueue()->interruptOnce();
    this->clientTable.getFromRemoteClientQueue().interruptOnce();

    Logger::logInfo("Stopping RoutingQueueMerger");
    status = this->routingQueueMerger->stop();
    if(!status.ok())
    {
        return status;
    }
    Logger::logInfo("RoutingQueueMerger stopped");



    if(this->remoteDispatcherServer != nullptr)
    {
        this->remoteDispatcherServer->shutdown();
    }

    if(this->remoteDispatcherClient != nullptr)
    {
        this->remoteDispatcherClient->shutdown();
    }


    Logger::logInfo("Stopping controlPackageHandler");
    running = false;
    if(this->controlPackageHandlerThread != nullptr)
    {
        this->moduleTable.controlPackagesQueue().interruptOnce();
        this->controlPackageHandlerThread->join();
        this->controlPackageHandlerThread = nullptr;
    }

    if(this->logMessageHandlerThread != nullptr)
    {
        this->logMessagesQueue->interruptOnce();
        this->logMessageHandlerThread->join();
        this->controlPackageHandlerThread = nullptr;
    }

    if(this->configUploadThread != nullptr)
    {
        this->configUploadThread->join();
        this->configUploadThread = nullptr;
    }

    
    Logger::logInfo("Middleware successfully shut down.");
    this->eventTracker->onCLAIDStopped();
    return absl::OkStatus();
}

absl::Status MiddleWare::populateModuleTable(
    const ModuleDescriptionMap& allModuleDescriptions,
    const ModuleDescriptionMap& hostModuleDescriptions,
    const ChannelDescriptionMap& channelDescriptions,
    ModuleTable& moduleTable)
{
    Logger::logInfo("Module size: %d", hostModuleDescriptions.size());
    for(auto& entry : allModuleDescriptions)
    {
        const ModuleDescription& moduleDescription = entry.second;
        moduleTable.setModuleChannelToConnectionMappings(moduleDescription.id, 
            moduleDescription.inputChannels, moduleDescription.outputChannels);
    }

    for(const auto& entry : hostModuleDescriptions)
    {
        const ModuleDescription& moduleDescription = entry.second;

        moduleTable.setNeededModule(moduleDescription.id,
            moduleDescription.moduleClass,
            moduleDescription.properties);
    }



    for(const auto& entry : channelDescriptions)
    {
        const ChannelDescription& channelDescription = entry.second;

        const std::string& channelName = channelDescription.channelName;

        for(const std::string& publisher : channelDescription.publisherModules)
        {
            if(channelDescription.subscriberModules.size() == 0)
            {
                // Channel without subscriber.
                moduleTable.setExpectedChannel(channelName, publisher, "");
                continue;
            }
            for(const std::string& subscriber : channelDescription.subscriberModules)
            {
                moduleTable.setExpectedChannel(channelName, publisher, subscriber);
            }
        }
    }

    return absl::OkStatus();
}

const std::string& MiddleWare::getSocketPath() const
{
    return socketPath;
}

const std::string& MiddleWare::getHostId() const
{
    return hostId;
}

const std::string& MiddleWare::getUserId() const
{
    return userId;
}

const std::string& MiddleWare::getDeviceId() const
{
    return deviceId;
}

bool MiddleWare::isConnectedToRemoteServer() const
{
    return this->remoteDispatcherClient != nullptr && this->remoteDispatcherClient->isConnected();
}

absl::Status MiddleWare::getRemoteClientStatus() const
{
    return this->remoteDispatcherClient != nullptr ?
         this->remoteDispatcherClient->getLastStatus() : 
            absl::UnavailableError("Status of RemoteDispatcherClient not available, because the RemoteDispatcherClient does not exist.");

}


void MiddleWare::readControlPackages()
{
    while(this->running)
    {
        std::shared_ptr<DataPackage> controlPackage = this->moduleTable.controlPackagesQueue().interruptable_pop_front();

        if(controlPackage == nullptr)
        {
            continue;
        }

        this->handleControlPackage(controlPackage);
    }
}

void MiddleWare::handleControlPackage(std::shared_ptr<DataPackage> controlPackage)
{
    Logger::logInfo("Middleware received control package %s from Runtime %s\n%s", CtrlType_Name(controlPackage->control_val().ctrl_type()).c_str(), Runtime_Name(controlPackage->control_val().runtime()).c_str(),
    messageToString(*controlPackage).c_str());

    switch(controlPackage->control_val().ctrl_type())
    {
        case CtrlType::CTRL_UNLOAD_MODULES_DONE:
        {
            Logger::logInfo("Received CTRL_UNLOAD_MODULES_DONE from Runtime %s", Runtime_Name(controlPackage->control_val().runtime()).c_str());
            this->numberOfRuntimesThatUnloadedModules++;

            Logger::logInfo("1");
            if(this->numberOfRuntimesThatUnloadedModules == this->moduleTable.getNumberOfRunningRuntimes())
            {
                Logger::logInfo("2");
                this->waitingForAllRuntimesToUnloadModules = false;
                Logger::logInfo("3");
            }
            break;
        }
        case CtrlType::CTRL_RESTART_RUNTIME_DONE:
        {
            Logger::logInfo("Received CTRL_RESTART_RUNTIME_DONE from Runtime %s", Runtime_Name(controlPackage->control_val().runtime()).c_str());
            this->numberOfRuntimesThatRestarted++;

            if(this->numberOfRuntimesThatRestarted == this->moduleTable.getNumberOfRunningRuntimes())
            {
                this->waitingForAllRuntimesToRestart = false;
            }  
            break;
        }
        case CtrlType::CTRL_REQUEST_MODULE_ANNOTATIONS:
        {
            Logger::logInfo("Received CTRL_REQUEST_MODULE_ANNOTATIONS from Runtime %s", Runtime_Name(controlPackage->control_val().runtime()).c_str());

            const std::map<std::string, ModuleAnnotation>& moduleAnnotations = this->moduleTable.getModuleAnnotations();
           

            std::shared_ptr<DataPackage> response = std::make_shared<DataPackage>();
            ControlPackage& ctrlPackage = *response->mutable_control_val();

            ctrlPackage.set_ctrl_type(CtrlType::CTRL_REQUEST_MODULE_ANNOTATIONS_RESPONSE);
            ctrlPackage.set_runtime(controlPackage->control_val().runtime());
            response->set_source_host(controlPackage->target_host());
            response->set_target_host(controlPackage->source_host());

            Logger::logInfo("Getting module annotations");
            for(auto& moduleAnnotation : moduleAnnotations)
            {
                Logger::logInfo("Getting module annotations for %s %d", moduleAnnotation.first.c_str(), 0);
                (*ctrlPackage.mutable_module_annotations())[moduleAnnotation.first] = moduleAnnotation.second;
            }

            this->masterInputQueue.push_back(response);

            break;
        }
        case CtrlType::CTRL_REQUEST_MODULE_ANNOTATIONS_RESPONSE:
        {
            this->forwardControlPackageToTargetRuntime(controlPackage);
            break;
        }
        case CtrlType::CTRL_UPLOAD_CONFIG_AND_DATA:
        {
            if(this->configUploadThread != nullptr)
            {
                if(!this->uploadingConfigFinished)
                {
                    Logger::logError("Received CTRL_UPLOAD_CONFIG_AND_DATA control message, which currently cannot be processed, since we are already\n"
                    "loading a new config right now (config reload thread busy).");
                    return;
                }
                else
                {
                    this->configUploadThread->join();
                    this->configUploadThread = nullptr;
                    this->uploadingConfigFinished = false;
                }
            }
            
            this->uploadingConfigFinished = false;
            this->configUploadPackage = *controlPackage;
            this->configUploadThread = make_unique<std::thread>([&]() {
                    this->handleUploadConfigAndPayloadMessage();
            });
            break;  
        }
        case CtrlType::CTRL_DEACTIVATE_NETWORK_CONNECTIONS:
        {
            Logger::logWarning("Disabling network connections due to CTRL_DEACTIVATE_NETWORK_CONNECTIONS %d %d", this->remoteDispatcherClient != nullptr, this->remoteDispatcherClient->isRunning());
            if(this->remoteDispatcherClient != nullptr && this->remoteDispatcherClient->isRunning())
            {
                Logger::logWarning("Calling RemoteDispatcherClient shutdown.");
                this->remoteDispatcherClient->shutdown();
            }
            if(this->remoteDispatcherServer != nullptr && this->remoteDispatcherServer->isRunning())
            {
                this->remoteDispatcherServer->shutdown();
            }
            break;
        }
        case CtrlType::CTRL_ACTIVATE_NETWORK_CONNECTIONS:
        {
            Logger::logWarning("Enabling network connections due to CTRL_DEACTIVATE_NETWORK_CONNECTIONS");
            if(this->remoteDispatcherClient != nullptr && !this->remoteDispatcherClient->isRunning())
            {
                absl::Status status = this->remoteDispatcherClient->start();
                if(!status.ok())
                {
                    Logger::logError("%s", absl::StrCat("Received ControlPackage with type CTRL_ACTIVATE_NETWORK_CONNECTIONS. ",
                    "Failed to activate RemoteDispatcherClient: ", status.ToString()).c_str());
                }
            }
            if(this->remoteDispatcherServer != nullptr && !this->remoteDispatcherServer->isRunning())
            {
                absl::Status status = this->remoteDispatcherServer->start();
                if(!status.ok())
                {
                    Logger::logError("%s", absl::StrCat("Received ControlPackage with type CTRL_ACTIVATE_NETWORK_CONNECTIONS. ",
                    "Failed to activate RemoteDispatcherServer: ", status.ToString()).c_str());
                }
            }
            break;
        }
        // case CtrlType::CTRL_CONNECTED_TO_REMOTE_SERVER:
        // case CtrlType::CTRL_DISCONNECTED_FROM_REMOTE_SERVER:
        // case CtrlType::CTRL_UNLOAD_MODULES:
        // case CtrlType::CTRL_PAUSE_MODULE:
        // case CtrlType::CTRL_UNPAUSE_MODULE:
        // case CtrlType::CTRL_ADJUST_POWER_PROFILE:
        case CtrlType::CTRL_REMOTE_FUNCTION_REQUEST:
        {
            const ControlPackage& ctrlPackage = controlPackage->control_val();
            const RemoteFunctionRequest& rpcRequest = ctrlPackage.remote_function_request();
            const RemoteFunctionIdentifier& remoteFunctionIdentifier = rpcRequest.remote_function_identifier();

            if(remoteFunctionIdentifier.function_type_case() == RemoteFunctionIdentifier::FunctionTypeCase::kRuntime)
            {
                if(remoteFunctionIdentifier.runtime() == Runtime::MIDDLEWARE_CORE)
                {
                    this->remoteFunctionRunnableHandler.executeRemoteFunctionRunnable(controlPackage);
                }
                else
                {
                    forwardControlPackageToSpecificRuntime(controlPackage, remoteFunctionIdentifier.runtime());
                }
            }
            else if(remoteFunctionIdentifier.function_type_case() == RemoteFunctionIdentifier::FunctionTypeCase::kModuleId)
            {
                const std::string& targetModule = remoteFunctionIdentifier.module_id();

                SharedQueue<DataPackage>* queue = this->moduleTable.lookupOutputQueue(targetModule);

                // Route package to target runtime.
                if(queue == nullptr)
                {
                    Logger::logError("Unable to forward RPC request. Unable to find Module %s", targetModule.c_str());
                    handleRPCModuleNotFoundError(controlPackage);
                    return;
                }
                queue->push_back(controlPackage);

            }
            // if(rpcRequest.)
            break;
        }
        case CtrlType::CTRL_REMOTE_FUNCTION_RESPONSE:
        {
          

            forwardControlPackageToSpecificRuntime(controlPackage, controlPackage->control_val().runtime());

            // if(remoteFunctionIdentifier.function_type_case() == RemoteFunctionIdentifier::FunctionTypeCase::kRuntime)
            // {
            //     // The convention is as follows: When a RPC targets a runtime (and not a module), the target
            //     // runtime (as specified by rpcRequest.remote_function_identifier().runtime()) sets dataPackage.ctrl_val().runtime() to the runtime
            //     // which was issuing the rpc request (as specified by the ctrl_val().runtime() value of the original RPC request data package).
            // }
            // else if(remoteFunctionIdentifier.function_type_case() == RemoteFunctionIdentifier::FunctionTypeCase::kModuleId)
            // {
            //     // The convention is as follows: When a RPC targets a module (and not a runtime), target_module of
            //     // the package is set to the module whose function to execute, and source_module

            //     const std::string& targetModule = controlPackage->target_module();
            //     SharedQueue<DataPackage>* queue = this->moduleTable.lookupOutputQueue(targetModule);
            //     // Route package to target runtime.
            //     queue->push_back(controlPackage);
            // }
            break;
        }
        case CtrlType::CTRL_DIRECT_SUBSCRIPTION_DATA:
        {
            this->forwardControlPackageToSpecificRuntime(controlPackage, 
                controlPackage->control_val().loose_direct_subscription().subscriber_runtime());
            break;
        }
        default:
        {
            this->forwardControlPackageToAllRuntimes(controlPackage);
            break;
        }

        
        /*case CtrlType::CTRL_LOCAL_LOG_MESSAGE:
        {
            std::shared_ptr<LogMessage> logMessage(new LogMessage(controlPackage->control_val().log_message()));
            this->logMessagesQueue->push_back(logMessage);
            break;
        }
        case CtrlType::CTRL_LOG_SINK_LOG_MESSAGE_STREAM:
        {
            this->forwardLogSinkLogStreamMessageToRuntimesThatSubscribed(controlPackage);
            break;
        }
        case CtrlType::CTRL_SUBSCRIBE_TO_LOG_SINK_LOG_MESSAGE_STREAM:
        {
            if(controlPackage->source_host() != this->hostId)
            {
                Logger::logError("A request was made by host \"%s\" to subscribe to the log stream messages of host \"%s\", which is not allowed. "
                "Only local runtimes can subscribe to the log stream messags of this host", 
                controlPackage->source_host().c_str(), this->hostId.c_str());
                return;
            }
            this->runtimesThatSubscribedToLogSinkStream.insert(controlPackage->control_val().runtime());
            break;
        }
        case CtrlType::CTRL_UNSUBSCRIBE_FROM_LOG_SINK_LOG_MESSAGE_STREAM:
        {
            if(controlPackage->source_host() != this->hostId)
            {
                Logger::logError("A request was made by host \"%s\" to unsubscribe to the log stream messages of host \"%s\", which is not allowed. "
                "Only local runtimes can subscribe to the log stream messags of this host", 
                controlPackage->source_host().c_str(), this->hostId.c_str());
                return;
            }

            auto it = this->runtimesThatSubscribedToLogSinkStream.find(controlPackage->control_val().runtime());
            if(it == this->runtimesThatSubscribedToLogSinkStream.end())
            {
                Logger::logWarning("Runtime %s tried to unsubscribe from log sink log messages stream, however the runtime never subscribed before",
                Runtime_Name(controlPackage->control_val().runtime()).c_str());
                return;
            }

            this->runtimesThatSubscribedToLogSinkStream.erase(it);
            break;
        }*/


    }
}

void MiddleWare::forwardControlPackageToAllRuntimes(std::shared_ptr<DataPackage> package)
{
    Logger::logInfo("Get runtime queues");
    auto allQueues = this->moduleTable.getRuntimeQueues();
    for(auto queue : allQueues)
    {
        Logger::logInfo("enqueueing");
        queue->push_back(package);
    }
}

void MiddleWare::forwardControlPackageToTargetRuntime(std::shared_ptr<DataPackage> package)
{
    std::shared_ptr<SharedQueue<DataPackage>> queue = this->moduleTable.getOutputQueueOfRuntime(package->control_val().runtime());
    
    if(queue != nullptr)
    {
        queue->push_back(package);
    }
    else
    {
        Logger::logError("Failed to lookup queue for Runtime %s. Cannot forward package %s",
        Runtime_Name(package->control_val().runtime()).c_str(), messageToString(*package).c_str());
    }
}

void MiddleWare::forwardControlPackageToSpecificRuntime(std::shared_ptr<DataPackage> package, Runtime runtime)
{
    std::shared_ptr<SharedQueue<DataPackage>> queue = this->moduleTable.getOutputQueueOfRuntime(runtime);
    
    if(queue != nullptr)
    {
        queue->push_back(package);
    }
}



absl::Status MiddleWare::unloadAllModulesInAllLocalRuntimes()
{
    if(this->waitingForAllRuntimesToUnloadModules)
    {
        return absl::UnavailableError("Cannot load a new config. We are currently waiting for all local Runtimes "
        "to unload their Modules (i.e., loadNewConfig() was called before but has not finished yet).");
    }

    this->waitingForAllRuntimesToUnloadModules = true;
    this->numberOfRuntimesThatUnloadedModules = 0;
    this->modulesUnloaded = false;


    std::shared_ptr<DataPackage> package = std::make_shared<DataPackage>();
    ControlPackage& ctrlPackage = *package->mutable_control_val();
    
    ctrlPackage.set_ctrl_type(CtrlType::CTRL_UNLOAD_MODULES);
    package->set_source_host(this->hostId);
    package->set_target_host(this->hostId);
    Logger::logInfo("Forwarding CTRL_UNLOAD_MODULES package to all local runtimes");
    this->forwardControlPackageToAllRuntimes(package);
            Logger::logInfo("4");

    while(this->waitingForAllRuntimesToUnloadModules)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
            Logger::logInfo("5");

    this->modulesUnloaded = true;
            Logger::logInfo("6");
    return absl::OkStatus();
}

absl::Status MiddleWare::getHostModuleAndChannelDescriptions(const std::string& hostId, const Configuration& config,
        HostDescriptionMap& hostDescriptions, ModuleDescriptionMap& allModuleDescriptions,
        ModuleDescriptionMap& hostModuleDescriptions, ChannelDescriptionMap& channelDescriptions)
{
    absl::Status status;
    status = config.getHostDescriptions(hostDescriptions);
    if (!status.ok()) {
        return status;
    }

    status = config.getModuleDescriptions(allModuleDescriptions);
    if (!status.ok()) {
        return status;
    }

    status = config.extractModulesForHost(hostId, allModuleDescriptions, hostModuleDescriptions);
    if (!status.ok()) {
        return status;
    }

    status = config.getChannelDescriptions(channelDescriptions);
    if (!status.ok()) {
        return status;
    }

    return absl::OkStatus();
}
    

absl::Status MiddleWare::loadNewConfigIntoModuleTableAndRouter(const Configuration& config)
{
    // This is ONLY safe to do, if all Modules in all Runtimes have been unloaded!!
    if(!this->modulesUnloaded)
    {
        return absl::UnavailableError("Cannot restart runtimes, have to unload Modules first!");
    }


    HostDescriptionMap hostDescriptions;

    // All Modules specified in the configuration file.
    // Needed by the Router.
    ModuleDescriptionMap allModuleDescriptions;

    // Subset of allModuleDescriptions. Contains only the Modules running on the current host.
    // Needed to populate the ModuleTable.
    ModuleDescriptionMap hostModuleDescriptions;

    ChannelDescriptionMap channelDescriptions;

    absl::Status status; 
    status = getHostModuleAndChannelDescriptions(hostId, config, hostDescriptions, allModuleDescriptions, hostModuleDescriptions, channelDescriptions);
    if (!status.ok()) {
        return status;
    }
   

    HostDescriptionMap oldHostDescriptions;
    status = this->currentConfiguration.getHostDescriptions(oldHostDescriptions);
    if(!status.ok())
    {
        return status;
    }

    
    if(hostDescriptions.size() != oldHostDescriptions.size())
    {
        return absl::Status(absl::InvalidArgumentError(absl::StrCat(
            "Unable to load new configuration into ModuleTable.\n",
            "The number of hosts has changed. Previously we had ", oldHostDescriptions.size(), " host,\n",
            "but now we have ", hostDescriptions.size()
        )));
    }

    for(const auto& entry : hostDescriptions)
    {
        const std::string& hostName = entry.first;
        auto it = oldHostDescriptions.find(hostName);
        if(it == oldHostDescriptions.end())
        {
            return absl::Status(absl::InvalidArgumentError(absl::StrCat(
                "Unable to load new configuration into ModuleTable.\n",
                "A new host with name \"", hostName, "\" was found, which is not available in the old host configuration.\n",
                "Adding new hosts while reloading configs is not supported. You can only change the Modules per host."
            )));
        }

        if(entry.second.connectTo != it->second.connectTo)
        {
            return absl::Status(absl::InvalidArgumentError(absl::StrCat(
                "Unable to load a new configuration into ModuleTable.\n",
                "The new config has made changes to the internal configuration for host \"", entry.first, "\".\n",
                "Previously, the property \"connectTo\" was set to \"", it->second.connectTo, " and now is \"", entry.second.connectTo, "\".\n",
                "Changing the internal configuration of hosts when reloading configs is not supported. You can only change the Modules per host.")));
        }

        if(entry.second.isServer != it->second.isServer)
        {
            return absl::Status(absl::InvalidArgumentError(absl::StrCat(
                "Unable to load a new configuration into ModuleTable.\n",
                "The new config has made changes to the internal configuration for host \"", entry.first, "\".\n",
                "Previously, the property \"isServer\" was set to \"", it->second.isServer, " and now is \"", entry.second.isServer, "\".\n",
                "Changing the internal configuration of hosts when reloading configs is not supported. You can only change the Modules per host.")));
        }

        if(entry.second.connectTo != it->second.connectTo)
        {
            return absl::Status(absl::InvalidArgumentError(absl::StrCat(
                "Unable to load a new configuration into ModuleTable.\n",
                "The new config has made changes to the internal configuration for host \"", entry.first, "\".\n",
                "Previously, the property \"hostServerAddress\" was set to \"", it->second.hostServerAddress, " and now is \"", entry.second.hostServerAddress, "\".\n",
                "Changing the internal configuration of hosts when reloading configs is not supported. You can only change the Modules per host.")));
        }
    }
        
    Logger::logInfo("Stopping Router.");
    status = this->masterRouter->stop();
    if(!status.ok())
    {
        return status;
    }
    Logger::logInfo("Updating Router.");
    status = this->masterRouter->updateHostAndModuleDescriptions(hostDescriptions, allModuleDescriptions);
    if(!status.ok())
    {
        return status;
    }
    Logger::logInfo("Starting router");
    status = this->masterRouter->start();
    if(!status.ok())
    {
        return status;
    }

    this->currentConfiguration = config;
    
    this->moduleTable.clearLookupTables();
    status = populateModuleTable(allModuleDescriptions, hostModuleDescriptions, channelDescriptions, moduleTable);
    if (!status.ok()) {
        return status;
    }
    return absl::OkStatus();
}


absl::Status MiddleWare::restartRuntimesWithNewConfig()
{
    this->waitingForAllRuntimesToRestart = true;
    this->numberOfRuntimesThatRestarted = 0;

    std::shared_ptr<DataPackage> package = std::make_shared<DataPackage>();
    ControlPackage& ctrlPackage = *package->mutable_control_val();
    
    ctrlPackage.set_ctrl_type(CtrlType::CTRL_RESTART_RUNTIME);
    package->set_source_host(this->hostId);
    package->set_target_host(this->hostId);
    this->forwardControlPackageToAllRuntimes(package);
    Logger::logInfo("Forwarding CTRL_RESTART_RUNTIME package to all local runtimes");

    while(this->waitingForAllRuntimesToUnloadModules)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    Logger::logInfo("Restarting runtimes done");

    return absl::OkStatus();
}

absl::Status MiddleWare::loadNewConfig(const std::string& configurationPath)
{
    Configuration config;
    Logger::logInfo("Parsing %s\n", configurationPath.c_str());
    auto status = config.parseFromJSONFile(configurationPath);
    if (!status.ok()) {
        Logger::logInfo("Unable to parse config file: %s", string(status.message()).c_str());
        return status;
    }

    this->configurationPath = configurationPath;

    return loadNewConfig(config);
}

absl::Status MiddleWare::loadNewConfig(const Configuration& config)
{
    Logger::logInfo("Unloading all Modules in all local runtimes.");

    // Loading a new config works as follows:
    // 1. All Runtimes stop and delete all Modules (= unloading them), and acknowledge it.
    // 2. Once all Runtimes have acknowledged that the Modules have been unloaded, we can clear
    // the lookup tables inside the ModuleTable and populate it again from the new config.
    // 3. Afterward, all Runtimes will restart and automatically receive the new configuration from the Middleware.
    // Loading a new config means all Modules will be stopped and deleted! It is not possible for information to persist 
    // between two configs (i.e., if there are two Modules with the same id before and after, they will still be completely restarted).
   
    this->setupLogSink();

    absl::Status status;
    status = this->unloadAllModulesInAllLocalRuntimes();
    if(!status.ok())
    {
        return status;
    }

    Logger::logInfo("Done unloading all Modules in all local runtimes.");
    

    status = this->loadNewConfigIntoModuleTableAndRouter(config);
    if(!status.ok())
    {
        return status;
    }

    status = this->restartRuntimesWithNewConfig();
    if(!status.ok())
    {
        return status;
    }


        


    // if(config.isDesignerModeEnabled())
    // {
    //     this->enableDesignerMode();
    // }
    // else
    // {
    //     this->disableDesignerMode();
    // }

    return absl::OkStatus();
}


void MiddleWare::readLocalLogMessages()
{
    while(this->running)
    {
        std::shared_ptr<LogMessage> logMessage = this->logMessagesQueue->interruptable_pop_front();

        if(logMessage == nullptr)
        {
            continue;
        }

        this->handleLocalLogMessage(logMessage);
    }
}

void MiddleWare::handleLocalLogMessage(std::shared_ptr<LogMessage> logMessage)
{
    std::unique_lock<std::mutex>(this->logSinkConfigurationMutex);

    if(!this->logSinkConfiguration.loggingToLogSinkEnabled())
    {
        return;
    }

    if(this->logSinkConfiguration.logSinkTransferMode == LogSinkTransferMode::STREAM)
    {
        std::shared_ptr<DataPackage> package = std::make_shared<DataPackage>();
        ControlPackage& ctrlPackage = *package->mutable_control_val();
        
        ctrlPackage.set_ctrl_type(CtrlType::CTRL_LOG_SINK_LOG_MESSAGE_STREAM);
        *ctrlPackage.mutable_log_message() = *logMessage;
        package->set_source_host(this->hostId);
        package->set_target_host(this->logSinkConfiguration.logSinkHost);
        this->masterInputQueue.push_back(package);
    }
    
}

void MiddleWare::forwardLogSinkLogStreamMessageToRuntimesThatSubscribed(std::shared_ptr<DataPackage> package)
{
    for(Runtime runtime : runtimesThatSubscribedToLogSinkStream)
    {
        std::shared_ptr<DataPackage> packageCopy(new DataPackage(*package));
        std::shared_ptr<SharedQueue<DataPackage>> queue = this->moduleTable.getOutputQueueOfRuntime(runtime);
    
        if(queue != nullptr)
        {
            queue->push_back(packageCopy);
        }
    }
}


void MiddleWare::setPayloadDataPath(const std::string& path)
{
    this->payloadDataPath = path;
}

const std::string& MiddleWare::getPayloadDataPath() const
{
    return this->payloadDataPath;
}

void MiddleWare::setCommonDataPath(const std::string& path)
{  
    Logger::logInfo("Setting common data path to %s, %lu", path.c_str(), this->eventTracker.get());
    this->commonDataPath = path;
    if(this->eventTracker != nullptr)
    {
        this->eventTracker->setStorageFolderPath(path);
    }
}

const std::string& MiddleWare::getCommonDataPath() const
{
    return this->commonDataPath;
}

void MiddleWare::enableDesignerMode()
{
    Logger::logInfo("Middleware enabled designer mdoe");
    this->designerModeActive = true;
}

void MiddleWare::disableDesignerMode()
{
    this->designerModeActive = false;
}

void MiddleWare::handleUploadConfigAndPayloadMessage()
{
    DataPackage& controlPackage = this->configUploadPackage;
    Logger::logInfo("got config package: %s", messageToString(controlPackage).c_str());
    const ConfigUploadPayload& payload = controlPackage.control_val().config_upload_payload();
    if(!this->designerModeActive)
    {
        Logger::logError("Received control package CTRL_UPLOAD_CONFIG_AND DATA from host \"%s\", "
        "however designer mode of this host (\"%s\") is disabled. Uploading a new config is forbidden.", controlPackage.source_host().c_str(), this->hostId.c_str());
        this->uploadingConfigFinished = true;
        return;
    }
    if(payload.payload_files().size() > 0)
    {
        if(this->payloadDataPath == "")
        {
            Logger::logError("Received control package CTRL_UPLOAD_CONFIG_AND DATA from host \"%s\", with additional payloads."
            "However payloadDataStoragePath of this host (\"%s\") is empty. Cannot load new config because we do not know where to store payload data.", controlPackage.source_host().c_str(), this->hostId.c_str());
            this->uploadingConfigFinished = true;
            return;
        }
        if(!storePayload(payload))
        {
            Logger::logError("Failed to store payload");
            this->uploadingConfigFinished = true;
            return;
        }
    }

    notifyAllRuntimesAboutNewPayload(payload);
    Configuration newConfig(payload.config());
    absl::Status status = loadNewConfig(newConfig);
    if(!status.ok())
    {
        Logger::logError("Failed to load new config: %s", status.ToString().c_str());
    }
    this->uploadingConfigFinished = true;
}

bool MiddleWare::storePayload(const ConfigUploadPayload& payload)
{
    for(const claidservice::DataFile& dataFile : payload.payload_files())
    {
        const std::string& relativePath = dataFile.relative_path();
        std::string folderPath;
        std::string filePath;
        Path::splitPathIntoFolderAndFileName(relativePath, folderPath, filePath);
        printf("folder file %s %s\n", folderPath.c_str(), filePath.c_str());
            
        if(folderPath != "")
        {
            std::string targetFolderPath = this->payloadDataPath + std::string("/") + folderPath;
            if(!FileUtils::dirExists(targetFolderPath))
            {
                if(!FileUtils::createDirectoriesRecursively(targetFolderPath))
                {
                    Logger::logError("%s", absl::StrCat("Error in DataReceiverModule, cannot create target folder \"", targetFolderPath, "\".").c_str());
                    return false;
                }
            }
        }

        std::string targetFilePath = 
                    this->payloadDataPath + std::string("/") + folderPath + std::string("/") + filePath;
        std::fstream file(targetFilePath, std::ios::out | std::ios::binary);
        if(!file.is_open())
        {
            Logger::logError("%s", absl::StrCat("Error, cannot save binary data to \"", targetFilePath, "\".\n. Could not open File for writing.").c_str());
            return false;
        }

        
        file.write(dataFile.file_data().data(), dataFile.file_data().size());
    }
    return true;
}

void MiddleWare::notifyAllRuntimesAboutNewPayload(const ConfigUploadPayload& payload)
{
    std::shared_ptr<DataPackage> package = std::make_shared<DataPackage>();
    ControlPackage& ctrlPackage = *package->mutable_control_val();
    
    ctrlPackage.set_ctrl_type(CtrlType::CTRL_ON_NEW_CONFIG_PAYLOAD_DATA);
    package->set_source_host(this->hostId);
    package->set_target_host(this->hostId);

    ConfigUploadPayload& payloadCopy = *ctrlPackage.mutable_config_upload_payload();
    payloadCopy = payload;
    payloadCopy.set_payload_data_path(this->payloadDataPath);


    this->forwardControlPackageToAllRuntimes(package);
    Logger::logInfo("Forwarding CTRL_ON_NEW_CONFIG_PAYLOAD_DATA package to all local runtimes");
}

int MiddleWare::getLogSinkSeverityLevel() const
{
    return static_cast<int>(this->logSinkConfiguration.logSinkSeverityLevel);
}


std::shared_ptr<EventTracker> MiddleWare::getEventTracker()
{
    return this->eventTracker;
}

void MiddleWare::handleRPCModuleNotFoundError(std::shared_ptr<DataPackage> rpcRequestPackage)
{
    const RemoteFunctionRequest& rpcRequest = rpcRequestPackage->control_val().remote_function_request();
    
    std::shared_ptr<DataPackage> response = std::make_shared<DataPackage>();
    ControlPackage* ctrlPackage = response->mutable_control_val();

    response->set_source_module(rpcRequestPackage->target_module());
    response->set_target_module(rpcRequestPackage->source_module());
    response->set_source_host(this->hostId);
    response->set_target_host(rpcRequestPackage->source_host());


    RemoteFunctionReturn* remoteFunctionReturn = ctrlPackage->mutable_remote_function_return();
    remoteFunctionReturn->set_execution_status(RemoteFunctionStatus::FAILED_MODULE_NOT_FOUND);
    remoteFunctionReturn->set_remote_future_identifier(rpcRequest.remote_future_identifier());
    *remoteFunctionReturn->mutable_remote_function_identifier() = rpcRequest.remote_function_identifier();

    ctrlPackage->set_ctrl_type(CtrlType::CTRL_REMOTE_FUNCTION_RESPONSE);
    
    ctrlPackage->set_runtime(rpcRequestPackage->control_val().runtime());

    forwardControlPackageToTargetRuntime(response);
}

std::map<std::string, std::string> MiddleWare::getAllRunningModulesOfAllRuntimes()
{
    return this->moduleTable.getModuleToClassMap();
}

bool MiddleWare::addLooseDirectSubscription(claidservice::LooseDirectChannelSubscription subscription)
{
    if(!this->moduleTable.isModulePublishingChannel(subscription.subscribed_module(), subscription.subscribed_channel()))
    {
        return false;
    }
    this->moduleTable.addLooseDirectSubscription(subscription);
    return true;
}

bool MiddleWare::addLooseDirectSubscriptionIfNotExists(claidservice::LooseDirectChannelSubscription subscription)
{
    if(!this->moduleTable.isModulePublishingChannel(subscription.subscribed_module(), subscription.subscribed_channel()))
    {
        return false;
    }
    this->moduleTable.addLooseDirectSubscriptionIfNotExists(subscription);
    return true;
}

void MiddleWare::removeLooseDirectSubscription(claidservice::LooseDirectChannelSubscription subscription)
{
    this->moduleTable.removeLooseDirectSubscription(subscription);
}