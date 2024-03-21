#include "dispatch/core/Module/ModuleManager.hh"

namespace claid {


ModuleManager::ModuleManager(DispatcherClient& dispatcher,
    SharedQueue<DataPackage>& fromModuleDispatcherQueue,
    SharedQueue<DataPackage>& toModuleDispatcherQueue,
    std::shared_ptr<EventTracker> eventTracker) :   dispatcher(dispatcher), 
                                                    fromModuleDispatcherQueue(fromModuleDispatcherQueue), 
                                                    toModuleDispatcherQueue(toModuleDispatcherQueue),
                                                    subscriberPublisher(toModuleDispatcherQueue),
                                                    eventTracker(eventTracker)
{
    Logger::logInfo("ModuleManager constructor event tracker %lu", eventTracker.get());
}

ModuleManager::~ModuleManager()
{
    if(this->restartThread)
    {
        this->restartThread->join();
        this->restartThread = nullptr;
    }
}

absl::Status ModuleManager::instantiateModule(const std::string& moduleId, const std::string& moduleClass)
{
    ModuleFactory& moduleFactory = *ModuleFactory::getInstance();

    if(!moduleFactory.isModuleClassRegistered(moduleClass))
    {
        return absl::NotFoundError(absl::StrCat("ModuleManager: Failed to instantiate Module of class \"", moduleClass, "\" (id: ", moduleId, "\")\n",
        "A Module with this class is not registered to the ModuleFactory."));
    
        // The Middleware always returns all Modules named in the config file.
        // If we cannot load it, we assume another registered runtime can.
        // Therefore, no error.
        //  return true;
    }

    Logger::logInfo("Loaded Module id \"%s\" (class: \"%s\").", moduleId.c_str(), moduleClass.c_str());
    std::unique_ptr<Module> moduleInstance(moduleFactory.getInstanceUntyped(moduleClass));

    moduleInstance->setId(moduleId);
    moduleInstance->setType(moduleClass);
    moduleInstance->setEventTracker(this->eventTracker);
    this->runningModules.insert(make_pair(moduleId, std::move(moduleInstance)));
    return absl::OkStatus();
}

absl::Status ModuleManager::instantiateModules(const ModuleListResponse& moduleList)
{
    for(ModuleListResponse_ModuleDescriptor descriptor : moduleList.descriptors())
    {
        const std::string& moduleId = descriptor.module_id();
        const std::string& moduleClass = descriptor.module_class();

        absl::Status status = this->instantiateModule(moduleId, moduleClass);
        if(!status.ok())
        {
            return status;
        }
    }

    return absl::OkStatus();
}

absl::Status ModuleManager::initializeModules(const ModuleListResponse& moduleList, ChannelSubscriberPublisher& subscriberPublisher)
{
    for(const ModuleListResponse_ModuleDescriptor& descriptor : moduleList.descriptors())
    {
        const std::string& moduleId = descriptor.module_id();
        const std::string& moduleClass = descriptor.module_class();

        const std::string& key = moduleId;
        if(this->runningModules.find(key) == this->runningModules.end())
        {
            return absl::NotFoundError(absl::StrCat(
                "Failed to initialize Module \"" + moduleId + "\" (class: \"" + moduleClass + "\").\n" +
                "The Module was not loaded."
            ));
            
            // The middleware always sends all Modules parsed from the configuration file.
            // If this Module is not running in this runtime, that means we could not instantiate it before.
            // Hence, we assume this Module is supported by another runtime.
            // continue;
        }

        std::unique_ptr<Module>& module = this->runningModules[key];

        
        // This will call the initialize functions of each Module.
        // In the initialize function (and ONLY there), Modules can publish or subscribe Channels.
        // Hence, once all Modules have been initialized, we know the available Channels.
        Logger::logInfo("Calling module.start() for Module \"%s\".", moduleId.c_str());
        
        Properties properties = descriptor.properties();

        if(!module->start(&subscriberPublisher, properties))
        {
            return absl::AbortedError(absl::StrCat(
                "Failed to start Module \"", moduleId.c_str(), "\""
            ));
        }
        Logger::logInfo("Module \"%s\" has started.",  moduleId.c_str());

    }
    return absl::OkStatus();
}

// A Channel is defined by a template package defining source | target | payload.
// For each Module, we have a list of DataPackages defining all Channels published or subscribed by the Module.
// This functions builds a Map<Module, List<DataPackage>>, representing the (data packages for each) channel of the Module.
void ModuleManager::getTemplatePackagesOfModules(std::map<std::string, std::vector<DataPackage>>& moduleChannels)
{
    moduleChannels.clear();
    for(const auto& entry : this->runningModules)
    {
        const std::string& moduleId = entry.first;

        std::vector<DataPackage> templatePackagesForModule = this->subscriberPublisher.getChannelTemplatePackagesForModule(moduleId);
        moduleChannels.insert(make_pair(moduleId, templatePackagesForModule));
    }
}

InitRuntimeRequest ModuleManager::makeInitRuntimeRequest()
{
    InitRuntimeRequest initRuntimeRequest;
    std::map<std::string, std::vector<DataPackage>> moduleTemplatePackages;
    getTemplatePackagesOfModules(moduleTemplatePackages);
    for(const auto& entry : moduleTemplatePackages)
    {
        InitRuntimeRequest_ModuleChannels moduleChannels;

        moduleChannels.set_module_id(entry.first);
        moduleChannels.clear_channel_packets();

        Logger::logInfo("Making init run time request %s, num channels: %d", entry.first.c_str(), entry.second.size());
        for(const DataPackage& package : entry.second)
        {
            *moduleChannels.add_channel_packets() = package;
        }

        *initRuntimeRequest.add_modules() = moduleChannels;
    }   
    initRuntimeRequest.set_runtime(Runtime::RUNTIME_CPP);
    return initRuntimeRequest;
}


absl::Status ModuleManager::start()
{
    Logger::logInfo("CLAID C++ ModuleManager start called");
    if(this->running)
    {
        return absl::AlreadyExistsError("Cannot start C++ Module manager. Start has been called twice.");
    }

    Logger::logInfo("CLAID C++ ModuleManager start called 2");

//    this->dispatcher = std::make_unique<DispatcherClient>(socketPath, fromModuleDispatcherQueue, toModuleDispatcherQueue, );
    std::unique_ptr<ModuleListResponse> moduleList = this->dispatcher.getModuleList();
    if(moduleList == nullptr)
    {
        return absl::AbortedError("Failed to receive ModuleListResponse from middleware.");
    }
        Logger::logInfo("CLAID C++ ModuleManager start called 3");
        Logger::logInfo("%s", messageToString(*moduleList).c_str());

    absl::Status status;
    Logger::logInfo("CLAID C++ ModuleManager start called 4");

    status = instantiateModules(*moduleList);
    if(!status.ok())
    {
        return status;
    }
    Logger::logInfo("CLAID C++ ModuleManager start called 5");


    status = initializeModules(*moduleList, subscriberPublisher);
    if(!status.ok())
    {
        return status;
    }


    InitRuntimeRequest initRuntimeRequest = makeInitRuntimeRequest();

    grpc::Status dispatcherStatus = this->dispatcher.startRuntime(initRuntimeRequest);
    if(!dispatcherStatus.ok())
    {
        return absl::Status(static_cast<absl::StatusCode>(dispatcherStatus.error_code()),
                       dispatcherStatus.error_message());
    }

    this->running = true;


    this->fromModuleDispatcherReaderThread = std::thread(&ModuleManager::readFromModulesDispatcher, this);

    Logger::logInfo("CLAID C++ runtime started successfully!");

    return absl::OkStatus();
}

void ModuleManager::shutdownModules()
{
    Logger::logInfo("ModuleManager shutting down Modules.");

    for(const auto& entry : this->runningModules)
    {
        const std::string& moduleName = entry.first;
        Module& module = *entry.second;
        Logger::logInfo("ModuleManager shutting down Module \"%s\".", moduleName.c_str());
        module.shutdown();
    }
    this->runningModules.clear();
}

void ModuleManager::stop()
{
    if(!this->running)
    {
        return;
    }

    Logger::logInfo("ModuleManager waiting for reader thread to stop.");

    this->shutdownModules();
    Logger::logInfo("All Modules shutdown in C++ runtime.");

    this->running = false;
    this->fromModuleDispatcherQueue.interruptOnce();
    this->fromModuleDispatcherReaderThread.join();
    Logger::logInfo("ModuleManager has stopped.");
    // Reset the subscriber publisher
    this->subscriberPublisher.reset();
}


void ModuleManager::readFromModulesDispatcher()
{
    while(this->running)
    {
        std::shared_ptr<DataPackage> pkt = fromModuleDispatcherQueue.interruptable_pop_front();

        if (!pkt) 
        {
            Logger::logInfo("ModuleManager received nullptr package from ModuleDispatcher");
            continue;
        }

        onPackageReceivedFromModulesDispatcher(pkt);
    }
}

void ModuleManager::onPackageReceivedFromModulesDispatcher(std::shared_ptr<DataPackage> dataPackage)
{
    /*String[] hostAndModule = splitHostModule(dataPackage.getTargetModule());

    if(hostAndModule == null)
    {
        Logger.logError("Java Runtime received DataPackage with invalid address \"" + dataPackage.getTargetModule() + "\".\n"
        + "Unable to split the address into host:module");
    }*/

    if(dataPackage->has_control_val())
    {
        Logger::logInfo("ModuleManager received DataPackage with controlVal %d (%s)", dataPackage->control_val().ctrl_type(), CtrlType_Name(dataPackage->control_val().ctrl_type()).c_str());
        handlePackageWithControlVal(dataPackage);
        return;
    }

    /*String hostName = hostAndModule[0];
    String moduleId = hostAndModule[1];*/

    const std::string& channelName = dataPackage->channel();
    const std::string& moduleId = dataPackage->target_module();

    Logger::logInfo("ModuleManager received package with target for Module \"%s\" on Channel \"%s\"",
                    moduleId.c_str(), channelName.c_str());


    if(!subscriberPublisher.isDataPackageCompatibleWithChannel(*dataPackage, moduleId))
    {
        const std::string payloadCaseName = dataPackagePayloadCaseToString(*dataPackage);
        Logger::logInfo("ModuleManager received package with target for Module \"%s\" on Channel \"%s\",\n"
            "however the data type of payload of the package did not match the data type of the Channel.\n"
            "Expected payload type \"%s\" but got \"%s\".",
            moduleId.c_str(), channelName.c_str(), payloadCaseName.c_str(), payloadCaseName.c_str());

        return;
    }

    std::vector<std::shared_ptr<AbstractSubscriber>> subscriberList 
        = this->subscriberPublisher.getSubscriberInstancesOfModule(channelName, moduleId);

    Logger::logInfo("ModuleManager invoking subscribers. Found %d subscribers.", subscriberList.size());
    for(std::shared_ptr<AbstractSubscriber> subscriber : subscriberList)
    {
        Logger::logInfo("Invoking subscriber %lu", subscriber.get());
        subscriber->onNewData(dataPackage);
    
    }
    Logger::logInfo("ModuleManager done invoking subscribers.", subscriberList.size());

}

void ModuleManager::handlePackageWithControlVal(std::shared_ptr<DataPackage> package)
{
    switch(package->control_val().ctrl_type())
    {
        case CtrlType::CTRL_CONNECTED_TO_REMOTE_SERVER:
        {
            for(auto& modulesEntry : this->runningModules)
            {
                modulesEntry.second->notifyConnectedToRemoteServer();
            }
            break;
        }
        case CtrlType::CTRL_DISCONNECTED_FROM_REMOTE_SERVER:
        {
            for(auto& modulesEntry : this->runningModules)
            {
                modulesEntry.second->notifyDisconnectedFromRemoteServer();
            }
            break;
        }
        case CtrlType::CTRL_UNLOAD_MODULES:
        {   
            Logger::logInfo("ModuleManager received ControlPackage with code CTRL_UNLOAD_MODULES");
            Logger::logInfo("Unloading modules!");
            this->shutdownModules();
            std::shared_ptr<DataPackage> response = std::make_shared<DataPackage>();
            ControlPackage& ctrlPackage = *response->mutable_control_val();
            
            ctrlPackage.set_ctrl_type(CtrlType::CTRL_UNLOAD_MODULES_DONE);
            ctrlPackage.set_runtime(Runtime::RUNTIME_CPP);
            response->set_source_host(package->target_host());
            response->set_target_host(package->source_host());
            toModuleDispatcherQueue.push_back(response);
            Logger::logInfo("Unloading modules done and acknowledgement will be sent out");
            break;
        }
        case CtrlType::CTRL_RESTART_RUNTIME:
        {   
            // Need to switch to separate thread, because stop would call fromModuleDispatcherReaderThread,
            // which is the same thread as the one we are currently in.
            if(this->restartThread)
            {
                this->restartThread->join();
            }
            this->restartThread = std::make_unique<std::thread>(&ModuleManager::restart, this);

            restartControlPackage = *package.get();
            
            
            break;
        }
        case CtrlType::CTRL_PAUSE_MODULE:
        {
            Logger::logWarning("Received pauise module %s", package->target_module().c_str());
            const std::string& targetModule = package->target_module();

            auto it = this->runningModules.find(targetModule);
            if(it == this->runningModules.end())
            {
                // Logger::logError("Got request to pause Module \"%s\", but Module with that id could not be found. Is it running?");
                return;
            }
                        Logger::logWarning("Received pauise module  2");

            it->second->pauseModule();
                                    Logger::logWarning("Received pauise module  3");
            break;
        }
        case CtrlType::CTRL_UNPAUSE_MODULE:
        {
            const std::string& targetModule = package->target_module();

            auto it = this->runningModules.find(targetModule);
            if(it == this->runningModules.end())
            {
                // Logger::logError("Got request to pause Module \"%s\", but Module with that id could not be found. Is it running?");
                return;
            }
            it->second->resumeModule();
            break;
        }
        case CtrlType::CTRL_ADJUST_POWER_PROFILE:
        {
            const std::string& targetModule = package->target_module();

            auto it = this->runningModules.find(targetModule);
            if(it == this->runningModules.end())
            {
                // Logger::logError("Got request to pause Module \"%s\", but Module with that id could not be found. Is it running?");
                return;
            }
            it->second->adjustPowerProfile(package->control_val().power_profile());
            break;
        }
        default:
        {
            Logger::logWarning("ModuleManager received package with unsupported control val %d", package->control_val().ctrl_type());
        }
        break;
    }
}

void ModuleManager::restart()
{
    Logger::logInfo("ModuleManager received ControlPackage with code CTRL_RESTART_RUNTIME");
    Logger::logInfo("ModuleManager stopping");
    this->stop();
    Logger::logInfo("ModuleManager stopping dispatcher");
    this->dispatcher.shutdown();
    Logger::logInfo("ModuleManager restarting");
    absl::Status status = this->start();
    if(!status.ok())
    {
        Logger::logError("Failed to restart: %s", status.ToString().c_str());
        return;
    }
    Logger::logInfo("...aaaaaandddd done!");

    std::shared_ptr<DataPackage> response = std::make_shared<DataPackage>();
    ControlPackage& ctrlPackage = *response->mutable_control_val();

    ctrlPackage.set_ctrl_type(CtrlType::CTRL_RESTART_RUNTIME_DONE);
    ctrlPackage.set_runtime(Runtime::RUNTIME_CPP);
    response->set_source_host(restartControlPackage.target_host());
    response->set_target_host(restartControlPackage.source_host());
    toModuleDispatcherQueue.push_back(response);
    
}

}