#pragma once

#include "dispatch/core/Router/Router.hh"

namespace claid
{

    // Forwards a package to either the LocalRouter, ClientRouter or ServerRouter, depending on the target of the package.
    class ServerRouter final : public Router
    {
        public: 

        ServerRouter()
        {
            
        }

        absl::Status routePackage(std::shared_ptr<DataPackage> dataPackage) override final
        {
            return absl::OkStatus();
        }
    };
}