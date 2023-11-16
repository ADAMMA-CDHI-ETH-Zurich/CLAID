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
  
        static std::unique_ptr<DispatcherClient> moduleDispatcher;
        static std::unique_ptr<ModuleManager> moduleManager;

        static bool started;
        static void* handle;

        

        // Starts the middleware and attaches to it.
        static bool start(const std::string& socketPath, const std::string& configFilePath, const std::string& hostId, const std::string& userId, const std::string& deviceId)
        {
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

            if(!attachCppRuntime())
            {
                return false;
            }

            started = true;

            return true;
        }

        // Attaches to the Middleware, but does not start it.
        // Assumes that the middleware is started in another language (e.g., C++ or Dart).
        // HAS to be called AFTER start is called in ANOTHER language.
        static bool attachCppRuntime()
        {
            if(started)
            {
                Logger::logError("CLAID middleware start was called twice!");
                return false;
            }

            // moduleDispatcher = make_unique<DispatcherClient>(socketPath);
            // moduleManager = make_unique<ModuleManager>(hostId, moduleDispatcher, factory);

            // return moduleManager->start();
            return false;
        }

    };

}
