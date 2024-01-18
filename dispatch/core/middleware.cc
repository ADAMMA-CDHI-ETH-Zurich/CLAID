#include "dispatch/core/middleware.hh"
#include "dispatch/core/Configuration/Configuration.hh"
#include "dispatch/core/CLAID.hh"
#include "dispatch/core/Logger/Logger.hh"
#include "dispatch/core/Router/RoutingTreeParser.hh"

using namespace claid;
using namespace std;

MiddleWare::MiddleWare(const string& socketPath, const string& configurationPath,
    const string& hostId, const string& userId, const string& deviceId)
        : socketPath(socketPath), configurationPath(configurationPath),
          hostId(hostId), userId(userId), deviceId(deviceId) 
    {
        moduleTable.setProperties(ModuleTableProperties{userId, deviceId});
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
        Logger::printfln("Error running shutdown in MiddleWare destructor: %s", string(status.message()).c_str());
    }
}

absl::Status MiddleWare::start() {

    if(running)
    {
        return absl::AbortedError("Failed to start middleware, middleware is already running. Middleware::start was called twice");
    }

    // Fill the module table fromt he config file
    Configuration config;
    Logger::printfln("Parsing %s\n", configurationPath.c_str());
    auto status = config.parseFromJSONFile(configurationPath);
    if (!status.ok()) {
        Logger::printfln("Unable to parse config file: %s", string(status.message()).c_str());
        return status;
    }

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

    status = populateModuleTable(hostModuleDescriptions, channelDescriptions, moduleTable);
    if (!status.ok()) {
        return status;
    }
    Logger::printfln("Module Table:");
    Logger::printfln("%s", moduleTable.toString().c_str());

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

    Logger::printfln("Middleware started.");

    running = true;

    this->controlPackageHandlerThread = std::make_unique<std::thread>([this]() {
        this->readControlPackages();
    });

    this->currentConfiguration = config;

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

absl::Status MiddleWare::shutdown()
{
    if(!running)
    {
        return absl::InvalidArgumentError("MiddleWare::shutdown called although middleware was not running.");
    }
    // TODO: remove once the router is added.
    Logger::printfln("Middleware shutdown called.");

    absl::Status status;
    Logger::printfln("Stopping MasterRouter");
    
    status = this->masterRouter->stop();
    if(!status.ok())
    {
        return status;
    }

    moduleTable.inputQueue().interruptOnce();
    this->hostUserTable.inputQueue()->interruptOnce();
    this->clientTable.getFromRemoteClientQueue().interruptOnce();

    Logger::printfln("Stopping RoutingQueueMerger");
    status = this->routingQueueMerger->stop();
    if(!status.ok())
    {
        return status;
    }
    Logger::printfln("RoutingQueueMerger stopped");



    if(this->remoteDispatcherServer != nullptr)
    {
        this->remoteDispatcherServer->shutdown();
    }

    if(this->remoteDispatcherClient != nullptr)
    {
        this->remoteDispatcherClient->shutdown();
    }


    Logger::printfln("Stopping controlPackageHandler");
    running = false;
    if(this->controlPackageHandlerThread != nullptr)
    {
        this->moduleTable.controlPackagesQueue().interruptOnce();
        this->controlPackageHandlerThread->join();
        this->controlPackageHandlerThread = nullptr;
    }

    
    Logger::printfln("Middleware successfully shut down.");
    return absl::OkStatus();
}

absl::Status MiddleWare::populateModuleTable(
    const ModuleDescriptionMap& moduleDescriptions,
    const ChannelDescriptionMap& channelDescriptions,
    ModuleTable& moduleTable)
{
    Logger::printfln("Module size: %d", moduleDescriptions.size());
    for(const auto& entry : moduleDescriptions)
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
    switch(controlPackage->control_val().ctrl_type())
    {
        case CtrlType::CTRL_CONNECTED_TO_REMOTE_SERVER:
        case CtrlType::CTRL_DISCONNECTED_FROM_REMOTE_SERVER:
        case CtrlType::CTRL_UNLOAD_MODULES:
        {
            this->forwardControlPackageToAllRuntimes(controlPackage);
            break;
        }
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

absl::Status MiddleWare::getHostModuleAndChannelDescriptions(const std::string& hostId, Configuration& config,
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
    

absl::Status MiddleWare::loadNewConfigIntoModuleTable(Configuration& config)
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
        
    this->currentConfiguration = config;
    
    this->moduleTable.clearLookupTables();
    status = populateModuleTable(hostModuleDescriptions, channelDescriptions, moduleTable);
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
    Logger::printfln("Parsing %s\n", configurationPath.c_str());
    auto status = config.parseFromJSONFile(configurationPath);
    if (!status.ok()) {
        Logger::printfln("Unable to parse config file: %s", string(status.message()).c_str());
        return status;
    }

    Logger::logInfo("Unloading all Modules in all local runtimes.");

    // Loading a new config works as follows:
    // 1. All Runtimes stop and delete all Modules (= unloading them), and acknowledge it.
    // 2. Once all Runtimes have acknowledged that the Modules have been unloaded, we can clear
    // the lookup tables inside the ModuleTable and populate it again from the new config.
    // 3. Afterward, all Runtimes will restart and automatically receive the new configuration from the Middleware.
    // Loading a new config means all Modules will be stopped and deleted! It is not possible for information to persist 
    // between two configs (i.e., if there are two Modules with the same id before and after, they will still be completely restarted).
    status = this->unloadAllModulesInAllLocalRuntimes();
    if(!status.ok())
    {
        return status;
    }

    Logger::logInfo("Done unloading all Modules in all local runtimes.");

    status = this->loadNewConfigIntoModuleTable(config);
    if(!status.ok())
    {
        return status;
    }
    this->configurationPath = configurationPath;

    status = this->restartRuntimesWithNewConfig();
    if(!status.ok())
    {
        return status;
    }

    

    return absl::OkStatus();
}
