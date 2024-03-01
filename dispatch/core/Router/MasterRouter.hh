#pragma once

#include "dispatch/core/Router/Router.hh"
#include "dispatch/core/Router/LocalRouter.hh"
#include "dispatch/core/Router/ServerRouter.hh"
#include "dispatch/core/Router/ClientRouter.hh"
#include "dispatch/core/Configuration/HostDescription.hh"
#include "dispatch/core/Configuration/ModuleDescription.hh"

namespace claid
{

    // Waits on an incoming queue in a separate thread and forwards the package to either the LocalRouter,
    // ClientRouter or ServerRouter, depending on the target host of the package.
    class MasterRouter final : public Router
    {
        private:

            // - A host can either be a Client or a Server.
            //      - A Client can only connect to a Server.
            //      - A Server can
            //           - accept connections of inoming Clients
            //           - connect to another Server, therefore acting as a Client for that Server, except if that Server is the root Server (initial node in connection tree). 
            // - Each Client has a local dispatcher (for local Modules) as well as a client dispatcher (to reach the server the client is a client of, hence upwards in the connection tree).
            // - Each Server has a local dispatcher and a server dispatcher to reach the connected clients connected to the server (hence downwards in the routing tree), and optionally
            //   can have a client dispatcher, to connect as client to another server ("up" in the connection tree).


            // Typically, the MasterRouter has 3 sub-routers:

            // A ClientRouter, which routes to another server the current runtime is connected to ("up" in the routing tree).
            // A ServerRouter to connected hosts ("down" in the routing tree). 
            // ServerRouter might forward the package to the connected users, or a specific user.
            // And a LocalRouter, routing packages to local modules.
            std::vector<std::shared_ptr<Router>> routers;

            // This map decides which subrouter to forward the package to.
            std::map<std::string /* adress = host */, std::shared_ptr<Router>> routingTable;

            // All incoming packages either from a local runtime, from the server or client will be gathered in this queue.
            // Incoming packages on this queue will then be routed to either the localRouter, serverRouter or clientRouter 
            // based on the target host.
            SharedQueue<DataPackage>& incomingQueue;

            std::unique_ptr<std::thread> processingThread;
            bool active = false;

            const std::string currentHost;
            
            HostDescriptionMap hostDescriptions;
            ModuleDescriptionMap moduleDescriptions;

            absl::Status lastError;

        private:    
   

            absl::Status buildRoutingTable(const std::string& currentHost, const HostDescriptionMap& hostDescriptions);

            void processQueue();

            absl::Status addPackageDestinationIfNotSet(std::shared_ptr<DataPackage> package) const;
            absl::Status addPackageSourceIfNotSet(std::shared_ptr<DataPackage> package) const;
            bool findHostOfModule(const std::string& module, std::string& hostOfModule) const;

        public:
        

            template<typename... RouterTypes>
            MasterRouter(
                const std::string& currentHost,
                const HostDescriptionMap& hostDescriptions,
                const ModuleDescriptionMap& moduleDescriptions,
                SharedQueue<claidservice::DataPackage>& incomingQueue, 
                std::shared_ptr<RouterTypes>... routersToRegister) : incomingQueue(incomingQueue), currentHost(currentHost), 
                                                                    hostDescriptions(hostDescriptions), moduleDescriptions(moduleDescriptions)
            {
                routers = {std::static_pointer_cast<Router>(routersToRegister)...};
            }

            absl::Status start() override final;
            absl::Status stop();
            absl::Status stopAfterQueueFinished();

            absl::Status getLastError();

            absl::Status routePackage(std::shared_ptr<DataPackage> dataPackage) override final;
            absl::Status updateHostAndModuleDescriptions(const HostDescriptionMap& hostDescriptions,
                const ModuleDescriptionMap& moduleDescriptions);
            bool canReachHost(const std::string& hostname) override final;
    };
}