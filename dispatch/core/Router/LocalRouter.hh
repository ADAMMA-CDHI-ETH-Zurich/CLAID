#pragma once

#include "dispatch/core/Router/Router.hh"
#include "dispatch/core/module_table.hh"
#include "dispatch/core/Logger/Logger.hh"

using claidservice::CtrlType;

namespace claid
{
    // The LocalRouter is invoked from the MasterRouter, if a package is targeted
    // for a Module on the current host (this host).
    // The LocalRouter then routes the package to the correct RuntimeDispatcher via the corresponding queues.

    class LocalRouter final : public Router
    {
        private:
            ModuleTable& moduleTable;
            const std::string currentHost;

            absl::Status routeControlPackage(std::shared_ptr<DataPackage> package);
            void routeToAllRuntimes(std::shared_ptr<DataPackage> package);

        public:
        
            LocalRouter(const std::string& currentHost, ModuleTable& moduleTable);

            absl::Status routePackage(std::shared_ptr<DataPackage> dataPackage) override final;

            bool canReachHost(const std::string& hostname) override final;

    };
}