#pragma once

#include "dispatch/core/Router/Router.hh"
#include "dispatch/core/Router/LocalRouter.hh"
#include "dispatch/core/Router/ServerRouter.hh"
#include "dispatch/core/Router/ClientRouter.hh"
#include "dispatch/core/Configuration/HostDescription.hh"
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


            std::shared_ptr<Router> localRouter;
            std::shared_ptr<Router> clientRouter;  // Routes to another server the current runtime is connected to ("up" in the routing tree).
            std::shared_ptr<Router> serverRouter;  // Routes to connected hosts ("down" in the routing tree). 
                                                   // ServerRouter might forward the package to the connected users, or a specific user.


            // This map decides whether to route a DataPackage locally, to a server we are connected to ("up") or to a client connected to us ("down").
            // If the module is on the current host, it will be routed locally.
            // If the module is not on the current host, we determine whether we can reach the host from here, meaning it is either 
            // a direct client of our server dispatcher, or a client of any of the clients of the ServerDispatcher (i.e., route downards).
            // If we cannot reach the host from here (i.e., it is not "below" us in the routing tree), we send it to the ClientDispatcher (meaning we are 
            // a client connected to another server, hence we are routing upwards in the tree).
            std::map<std::string /* adress = host:module */, std::shared_ptr<Router>> routingTable;

            // The routing tree resulting from the configuration file.
            // This tree will be the same on each host, since they all share the same configuration file.
            // From the routingTree, we can generate the routingTable, which is specific for each host.
            RoutingTree routingTree;

            // All incoming packages either from a local runtime, from the server or client will be gathered in this queue.
            // Incoming packages on this queue will then be routed to either the localRouter, serverRouter or clientRouter 
            // based on the target host.
            SharedQueue<DataPackage>& incomingQueue;

            std::unique_ptr<std::thread> processingThread;
            bool active = false;

        private:    
            absl::Status buildRoutingTableFromTree(
                const std::string& currentHost, const HostDescriptionMap& hostDescriptions);

            void processQueue();

        public:
        

            MasterRouter(
                SharedQueue<claidservice::DataPackage>& incomingQueue, 
                std::shared_ptr<LocalRouter> localRouter,
                std::shared_ptr<ClientRouter> clientRouter,
                std::shared_ptr<ServerRouter> serverRouter);

            absl::Status buildRoutingTable(std::string currentHost, const HostDescriptionMap& hostDescriptions);

            absl::Status start() override final;
            absl::Status routePackage(std::shared_ptr<DataPackage> dataPackage) override final;
    };
}