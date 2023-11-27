#ifndef MIDDLEWARE_HH__
#define MIDDLEWARE_HH__

#include <string>
#include <memory>
#include <thread>

#include "absl/status/status.h"
#include "dispatch/core/module_table.hh"
#include "dispatch/core/local_dispatching.hh"
#include "dispatch/core/Configuration/Configuration.hh"

#include "dispatch/core/RemoteDispatching/RemoteDispatcherClient.hh"
#include "dispatch/core/RemoteDispatching/RemoteDispatcherServer.hh"
#include "dispatch/core/RemoteDispatching/HostUserTable.hh"
#include "dispatch/core/RemoteDispatching/ClientTable.hh"
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
                const std::string& hostId,
                const std::string& userId,
                const std::string& deficeId
            );

            absl::Status start();
           
            absl::Status shutdown();

            const std::string& getSocketPath() const;

            virtual ~MiddleWare();
        private:

            bool running = false;

            // TODO: Incorporate all variables into the code if necessary.
            std::string socketPath;
            std::string configurationPath;
            std::string hostId;
            std::string userId;
            std::string deviceId;

            ModuleTable moduleTable;
            HostUserTable hostUserTable;
            ClientTable clientTable;
                
            // Server for local runtimes (C++, Dart, Java, Python, ...) to connect to.
            std::unique_ptr<DispatcherServer> localDispatcher;

            // Server for external instances of CLAID to connect to.
            std::unique_ptr<RemoteDispatcherServer> remoteDispatcherServer;

            // Client for the current instance of CLAID to connect to another instance of CLAID.
            std::unique_ptr<RemoteDispatcherClient> remoteDispatcherClient;

            RoutingTree routingTree;

            // 
            std::unique_ptr<MasterRouter> masterRouter;
            
            // Merges the queus from the localDispatcher, remoteDispatcherServer and remoteDispatcherClient into a single queue.
            // This queue is processed by the masterRouter, which decides whether to route the package locally (to a runtime),
            // to a connected client (downwards in the routing tree) or to a connected server (upwards in the routing tree).
            std::unique_ptr<RoutingQueueMerger> routingQueueMerger;
            SharedQueue<DataPackage> masterInputQueue;

            absl::Status populateModuleTable(
                const ModuleDescriptionMap& moduleDescriptions,
                const ChannelDescriptionMap& channelDescriptions,
                claid::ModuleTable& moduleTable);

            absl::Status startRemoteDispatcherServer(const std::string& currentHost, const HostDescriptionMap& hostDescriptions);
            
            absl::Status startRemoteDispatcherClient(const std::string& currentHost, const std::string& currentUser, 
                const std::string& currentDeviceId, const HostDescriptionMap& hostDescriptions);
            
            absl::Status startRouter(const std::string& currentHost, const HostDescriptionMap& hostDescriptions, const ModuleDescriptionMap& moduleDescriptions);


    };
}

#endif    //  MIDDLEWARE_HH__

