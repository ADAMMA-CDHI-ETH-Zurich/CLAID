#pragma once

#include "dispatch/core/CLAIDConfig/CLAIDConfig.hh"
#include "dispatch/proto/claidservice.pb.h"
#include "dispatch/core/Router/DispatcherStub.hh"
#include "dispatch/core/Router/RoutingNode.hh"
#include "dispatch/core/Router/RoutingTree.hh"


#include <map>

using claidservice::CLAIDConfig;
using claidservice::HostConfig;
using claidservice::DataPackage;

namespace claid
{

    class Router
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


            Dispatcher* localDispatcher;
            Dispatcher* clientDispatcher; // Where the current runtime is connected to another server ("up" in the routing tree).
            Dispatcher* serverDispatcher;  // Where other clients can connect to this runtime ("down" in the routing tree). 
                                          // ServerDispatcher might forward the package to the connected users, or a specific user.


            // This map decides whether to route a DataPackage locally, to a server we are connected to ("up") or to a client connected to us ("down").
            // If the module is on the current host, it will be routed locally.
            // If the module is not on the current host, we determine whether we can reach the host from here, meaning it is either 
            // a direct client of our server dispatcher, or a client of any of the clients of the ServerDispatcher (i.e., route downards).
            // If we cannot reach the host from here (i.e., it is not "below" us in the routing tree), we send it to the ClientDispatcher (meaning we are 
            // a client connected to another server, hence we are routing upwards in the tree).
            std::map<std::string /* adress = host:module */, Dispatcher*> routingTable;

            // The routing tree resulting from the configuration file.
            // This tree will be the same on each host, since they all share the same configuration file.
            // From the routingTree, we can generate the routingTable, which is specific for each host.
            RoutingTree routingTree;

            void buildRoutingTableFromTree(
                const CLAIDConfig& config, const std::string& currentHost);

        public:
            void buildRoutingTable(std::string currentHost, const CLAIDConfig& config);
            bool routePackage(const DataPackage& dataPackage);
    };
}