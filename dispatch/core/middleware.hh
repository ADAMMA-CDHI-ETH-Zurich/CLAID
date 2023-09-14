#ifndef MIDDLEWARE_HH__
#define MIDDLEWARE_HH__

#include <string>
#include <memory>
#include <thread>

#include "absl/status/status.h"
#include "dispatch/core/module_table.hh"
#include "dispatch/core/local_dispatching.hh"

namespace claid
{
    // Top container of the CLAID middleware.
    // TODO: This should probably merged into CLAID.*
    class MiddleWare {
        public:
            MiddleWare(
                const std::string& socketPath,
                const std::string& configurationPath,
                const std::string& currentHost,
                const std::string& currentUser,
                const std::string& currentDeviceId
            );

            absl::Status start();
            absl::Status shutdown();

            virtual ~MiddleWare();
        private:
            // TODO: Incorporate all variables into the code if necessary.
            std::string socketPath;
            std::string configurationPath;
            std::string currentHost;
            std::string currentUser;
            std::string currentDeviceId;

            ModuleTable moduleTable;
            std::unique_ptr<DispatcherServer> localDispatcher;

            // TODO: Remove this, for temporary testing only
            std::unique_ptr<std::thread> routerThread;

    };
}

#endif    //  MIDDLEWARE_HH__

