#pragma once

#include "dispatch/core/Configuration/Configuration.hh"
#include "dispatch/core/Router/MasterRouter.hh"
#include "dispatch/core/Module/ModuleManager.hh"
#include "dispatch/core/middleware.hh"
namespace claid
{
    class CLAID
    {

        private:
  
        static ModuleDispatcher moduleDispatcher;
        static ModuleManager moduleManager;

        static boolean started = false;
        static long handle;

        

        // Starts the middleware and attaches to it.
        protected static boolean startInternal(final String socketPath, final String configFilePath, final String hostId, final String userId, final String deviceId, ModuleFactory moduleFactory)
        {
            if(started)
            {
                Logger.logError("CLAID middleware start was called twice!");
                return false;
            }
            
            handle = startCore(socketPath, configFilePath, hostId, userId, deviceId);
            
            if(handle == 0)
            {
                Logger.logError("Failed to start CLAID middleware core (start_core), returned handle is 0.");
                return false;
            }

            if(!startAttachInternal(socketPath, hostId, moduleFactory))
            {
                return false;
            }

            started = true;

            return true;
        }

        // Attaches to the Middleware, but does not start it.
        // Assumes that the middleware is started in another language (e.g., C++ or Dart).
        // HAS to be called AFTER start is called in ANOTHER language.
        protected static boolean startAttachInternal(final String socketPath, final String hostId, ModuleFactory factory)
        {
            if(started)
            {
                Logger.logError("CLAID middleware start was called twice!");
                return false;
            }

            moduleDispatcher = new ModuleDispatcher(socketPath);
            moduleManager = new ModuleManager(hostId, moduleDispatcher, factory);

            return moduleManager.start();
        }

            absl::Status start(const std::string& configurationPath, const std::string& currentHost);
            absl::Status attachCppRuntime();
        };

}
