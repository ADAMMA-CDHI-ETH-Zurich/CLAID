#pragma once

#include "dispatch/core/Router/Router.hh"
#include "dispatch/core/Router/RoutingTree.hh"
#include "dispatch/core/RemoteDispatching/HostUserTable.hh"
namespace claid
{

    // Forwards a package to either the LocalRouter, ClientRouter or ServerRouter, depending on the target of the package.
    class ServerRouter final : public Router
    {
    

    public: 
// std::shared_ptr<const RoutingTree> routingTree, 
        ServerRouter(const std::string& currentHost, const RoutingTree& routingTree, HostUserTable& hostUserTable);

        absl::Status routePackage(std::shared_ptr<DataPackage> dataPackage) override final;

        bool canReachHost(const std::string& hostname) override final;

    private:
            const std::string currentHost;
            const RoutingTree& routingTree;
            HostUserTable& hostUserTable;

            std::map<std::string /* target host */, 
                    std::vector<std::string> > /* route to host from currentHost */
                     routingTable;
        
    };
}