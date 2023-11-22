#pragma once

#include "dispatch/core/Router/Router.hh"
#include "dispatch/core/Router/RoutingTree.hh"
#include "dispatch/core/RemoteDispatching/ClientTable.hh"

namespace claid
{

    // Forwards a package to either the LocalRouter, ClientRouter or ServerRouter, depending on the target of the package.
    class ClientRouter final : public Router
    {
    public:

        ClientRouter(const std::string& currentHost,
            const RoutingTree& routingTree, ClientTable& clientTable);

        absl::Status routePackage(std::shared_ptr<DataPackage> dataPackage) override final;

        bool canReachHost(const std::string& hostname) override final;

    private:
        const std::string currentHost;
        const RoutingTree& routingTree;
        ClientTable& clientTable;
    };
}