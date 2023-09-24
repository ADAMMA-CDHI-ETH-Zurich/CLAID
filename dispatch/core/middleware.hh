#ifndef MIDDLEWARE_HH__
#define MIDDLEWARE_HH__

#include <string>
#include <memory>
#include <thread>

#include "absl/status/status.h"
#include "dispatch/core/module_table.hh"
#include "dispatch/core/local_dispatching.hh"
#include "dispatch/core/RemoteDispatching/RemoteDispatcherClient.hh"
#include "dispatch/core/RemoteDispatching/RemoteDispatcherServer.hh"
#include "dispatch/core/RemoteDispatching/HostUserTable.hh"
#include "dispatch/core/Router/MasterRouter.hh"
#include "dispatch/core/Router/LocalRouter.hh"
#include "dispatch/core/Router/ClientRouter.hh"
#include "dispatch/core/Router/ServerRouter.hh"
#include "dispatch/core/Router/RoutingQueueMerger.hh"
#include "dispatch/core/Router/RoutingQueueMergerGeneric.hh"

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
            absl::Status startRemoteDispatcherServer(const std::string& currentHost, const HostDescriptionMap& hostDescriptions);
            
            absl::Status startRemoteDispatcherClient(const std::string& currentHost, const std::string& currentUser, 
                const std::string& currentDeviceId, const HostDescriptionMap& hostDescriptions);
            
            absl::Status startRouter(const std::string& currentHost, const HostDescriptionMap& hostDescriptions);
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
            HostUserTable hostUserTable;
            std::unique_ptr<DispatcherServer> localDispatcher;
            std::unique_ptr<RemoteDispatcherServer> remoteDispatcherServer;
            std::unique_ptr<RemoteDispatcherClient> remoteDispatcherClient;

            std::unique_ptr<MasterRouter> masterRouter;
            std::unique_ptr<RoutingQueueMerger> routingQueueMerger;
            SharedQueue<DataPackage> masterInputQueue;


            // TODO: Remove this, for temporary testing only
            std::unique_ptr<std::thread> routerThread;

    };
}

#endif    //  MIDDLEWARE_HH__

