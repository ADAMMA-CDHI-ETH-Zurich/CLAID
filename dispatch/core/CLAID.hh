#pragma once

#include "dispatch/core/Configuration/Configuration.hh"
#include "dispatch/core/Router/MasterRouter.hh"
#include "dispatch/core/Module/ModuleManager.hh"
#include "dispatch/core/local_dispatching.hh"
#include "dispatch/core/middleware.hh"

#include "dispatch/core/capi.h"
namespace claid
{
    class CLAID
    {

    private:
  
        std::unique_ptr<DispatcherClient> moduleDispatcher;
        std::unique_ptr<ModuleManager> moduleManager;

        SharedQueue<DataPackage> fromModuleDispatcherQueue;
        SharedQueue<DataPackage> toModuleDispatcherQueue;

        bool started = false;
        void* handle;

    public:

        // Starts the middleware and attaches to it.
        bool start(const std::string& socketPath, const std::string& configFilePath, const std::string& hostId, const std::string& userId, const std::string& deviceId);
        
        bool shutdown();

        // Attaches to the Middleware, but does not start it.
        // Assumes that the middleware is started in another language (e.g., C++ or Dart).
        // HAS to be called AFTER start is called in ANOTHER language.
        bool attachCppRuntime(void* handle);

        bool isConnectedToRemoteServer() const;
        absl::Status getRemoteClientStatus() const;
    };

}
