#pragma once

#include "dispatch/core/Router/Router.hh"
#include "dispatch/core/Router/RoutingTree.hh"

namespace claid
{

    // Forwards a package to either the LocalRouter, ClientRouter or ServerRouter, depending on the target of the package.
    class ClientRouter final : public Router
    {
    public:

        ClientRouter(const std::string& currentHost, 
            const RoutingTree& routingTree) : currentHost(currentHost), routingTree(routingTree)
        {
            
        }

        absl::Status routePackage(std::shared_ptr<DataPackage> dataPackage) override final
        {
            return absl::OkStatus();
        }

        bool canReachHost(const std::string& hostname) override final
        {
            std::vector<std::string> route;
            RoutingDirection direction;

            // Returns false if no route was found.
            if(!routingTree.getRouteFromHostToHost(this->currentHost, hostname, route, direction))
            {
                return false;
            }

            // We are a ClientRouter and route packages to a server we are connected to.
            // Hence, we can only route upwards in the routing tree.
            return direction == ROUTE_UP;
        }

    private:
        const std::string currentHost;
        const RoutingTree& routingTree;
    };
}