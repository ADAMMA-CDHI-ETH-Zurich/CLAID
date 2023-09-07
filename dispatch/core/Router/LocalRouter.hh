#pragma once

#include "dispatch/core/Router/Router.hh"

namespace claid
{
    // The LocalRouter is invoked from the MasterRouter, if a package is targeted
    // for a Module on the current host (this host). 
    // The LocalRouter then routes the package to the correct RuntimeDispatcher via the corresponding queues.

    class LocalRouter final : public Router
    {
        void buildRoutingTable(std::string currentHost, const CLAIDConfig& config) const override final
        {

        }
    };
}