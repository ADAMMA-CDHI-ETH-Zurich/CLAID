#include "dispatch/core/Router/ClientRouter.hh"

#include "absl/status/status.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_cat.h"

namespace claid {
    
    ClientRouter::ClientRouter(const std::string& currentHost,
                const RoutingTree& routingTree, ClientTable& clientTable) : currentHost(currentHost), routingTree(routingTree), clientTable(clientTable)
    {
        
    }

    absl::Status ClientRouter::routePackage(std::shared_ptr<DataPackage> dataPackage)
    {
        const std::string& sourceHost = dataPackage->source_host();
        const std::string& sourceModule = dataPackage->source_module();

        const std::string& targetHost = dataPackage->target_host();
        const std::string& targetModule = dataPackage->target_module();

        Logger::logInfo("ClientRouter routing package from host \"%s\" (Module \"%s\"), "
                        "destined for host \"%s\" (Module \"%s\").", sourceHost.c_str(), sourceModule.c_str(), targetHost.c_str(), targetModule.c_str());

        if(!canReachHost(targetHost))
        {
            return absl::InvalidArgumentError(absl::StrCat(
                "ClientRouter on host \"", currentHost, "\" failed to route package from host \"", sourceHost, "\" ",
                "to target host \"", targetHost, "\". There is no route from host \"", currentHost, "\" to host \"", targetHost, "\"."
            ));
        }

        this->clientTable.getQueue().push_back(dataPackage);

        return absl::OkStatus();
    }

    bool ClientRouter::canReachHost(const std::string& hostname)
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

}