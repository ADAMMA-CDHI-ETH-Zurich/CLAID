#include "dispatch/core/Router/ServerRouter.hh"

namespace claid
{
    ServerRouter::ServerRouter(const std::string& currentHost, 
        const RoutingTree& routingTree, 
        HostUserTable& hostUserTable) : currentHost(currentHost), routingTree(routingTree), hostUserTable(hostUserTable)
    {

    }

    absl::Status ServerRouter::routePackage(std::shared_ptr<DataPackage> dataPackage) 
    {
        const std::string& targetHost = dataPackage->target_host();
        
        if(!canReachHost(targetHost))
        {
            return absl::InvalidArgumentError(absl::StrCat(
                "Error in ServerRouter: Cannot route package from source host \"", dataPackage->source_host(), "\" ",
                "to target host \"", targetHost, "\". The target host is not connected as client to the source host."
            ));
        }

        // The function canReachHost will automatically cache the route to the targetHost in our routingTable.

        const std::vector<std::string>& route = this->routingTable[targetHost];



        return absl::OkStatus();
    }

    bool ServerRouter::canReachHost(const std::string& hostname)
    {
        // Check if we have seen this host already before. 
        // If yes, and we can reach it, we have cached the routing path to that host in our routing table.
        auto it = this->routingTable.find(hostname);
        if(it != this->routingTable.end())
        {
            return true;
        }

        std::vector<std::string> route;
        RoutingDirection direction;

        // Returns false if no route was found.
        if(!routingTree.getRouteFromHostToHost(this->currentHost, hostname, route, direction))
        {
            return false;
        }

        // We are a ServerRouter and route packages to clients that are connected to us.
        // Hence, we can only route downwards in the routing tree.
        if(direction != ROUTE_DOWN)
        {
            return false;
        }

        // Cache result for later routing.
        this->routingTable[hostname] = route;
        return true;
    }

}