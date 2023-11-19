#pragma once

#include "dispatch/core/Router/Router.hh"
#include "dispatch/core/module_table.hh"
#include "dispatch/core/Logger/Logger.hh"
namespace claid
{
    // The LocalRouter is invoked from the MasterRouter, if a package is targeted
    // for a Module on the current host (this host).
    // The LocalRouter then routes the package to the correct RuntimeDispatcher via the corresponding queues.

    class LocalRouter final : public Router
    {
        private:
            ModuleTable& moduleTable;

        public:
            LocalRouter(ModuleTable& moduleTable) : moduleTable(moduleTable)
            {

            }

            absl::Status routePackage(std::shared_ptr<DataPackage> dataPackage) override final
            {
                const std::string& targetModule = dataPackage->target_module();

                std::string targetHost;

                

             

                SharedQueue<DataPackage>* inputQueueForRuntime = this->moduleTable.lookupOutputQueue(targetModule);

                if(!inputQueueForRuntime)
                {
                    return absl::InvalidArgumentError(absl::StrCat("LocalRouter: Failed to route package to local Module \"", targetModule,"\".\n"
                    "Unable to get input queue of the Runtime the Module is running in. Possibly, the Runtime was not registered."));
                }

                inputQueueForRuntime->push_back(dataPackage);
                return absl::OkStatus();
            }

    };
}