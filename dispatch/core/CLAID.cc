#include "dispatch/core/CLAID.hh"
#include "dispatch/core/Router/RoutingQueueMergerGeneric.hh"
#include "dispatch/core/Router/MasterRouter.hh"
#include "dispatch/core/Configuration/Configuration.hh"
#include "dispatch/core/RemoteDispatching/HostUserTable.hh"
#include "dispatch/core/Logger/Logger.hh"


namespace claid {
    bool CLAID::started = false;

    std::unique_ptr<DispatcherClient> CLAID::moduleDispatcher;
    std::unique_ptr<ModuleManager> CLAID::moduleManager;

    SharedQueue<DataPackage> CLAID::fromModuleDispatcherQueue;
    SharedQueue<DataPackage> CLAID::toModuleDispatcherQueue;

    void* CLAID::handle = nullptr;
    
    bool CLAID::start(const std::string& socketPath, const std::string& configFilePath, const std::string& hostId, const std::string& userId, const std::string& deviceId)
    {
        Logger::logInfo("CLAID start");

        if(started)
        {
            Logger::logError("CLAID middleware start was called twice!");
            return false;
        }
        
        handle = start_core(socketPath.c_str(), configFilePath.c_str(), hostId.c_str(), userId.c_str(), deviceId.c_str());
        
        if(handle == 0)
        {
            Logger::logError("Failed to start CLAID middleware core (start_core), returned handle is 0.");
            return false;
        }

        if(!attachCppRuntime(handle))
        {
            return false;
        }

        started = true;

        return true;
    }

    // Attaches to the Middleware, but does not start it.
    // Assumes that the middleware is started in another language (e.g., C++ or Dart).
    // HAS to be called AFTER start is called in ANOTHER language.
    bool CLAID::attachCppRuntime(void* handle)
    {
        if(started)
        {
            Logger::logError("CLAID middleware start was called twice!");
            return false;
        }

        claid::MiddleWare* middleware = static_cast<claid::MiddleWare*>(handle);

        const std::string& socketPath = middleware->getSocketPath();

        const std::set<std::string> registeredModuleClasses = ModuleFactory::getInstance()->getRegisteredModuleClasses();

        moduleDispatcher = make_unique<DispatcherClient>(socketPath, fromModuleDispatcherQueue, toModuleDispatcherQueue, registeredModuleClasses);
        moduleManager = make_unique<ModuleManager>(*moduleDispatcher, fromModuleDispatcherQueue, toModuleDispatcherQueue);

        absl::Status status = moduleManager->start();

        if(!status.ok())
        {
            std::stringstream ss;
            ss << status;
            Logger::logFatal("%s", ss.str().c_str());
            return false;
        }
        
        started = true;
        return true;
    }

    bool CLAID::shutdown()
    {
        if(handle == nullptr)
        {
            return false;
        }   
        moduleManager->stop();
        moduleDispatcher->shutdown();

        shutdown_core(handle);
        
        moduleDispatcher = nullptr;
        moduleManager = nullptr;
        return true;
    }
}