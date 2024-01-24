#include "dispatch/core/Router/LocalRouter.hh"

namespace claid {

    LocalRouter::LocalRouter(const std::string& currentHost, ModuleTable& moduleTable) : moduleTable(moduleTable), currentHost(currentHost)
    {

    }

    absl::Status LocalRouter::routePackage(std::shared_ptr<DataPackage> dataPackage)
    {
        if(dataPackage->has_control_val())
        {
            return routeControlPackage(dataPackage);
        }
        const std::string& sourceHost = dataPackage->source_host();
        const std::string& sourceModule = dataPackage->source_module();

        const std::string& targetHost = dataPackage->target_host();
        const std::string& targetModule = dataPackage->target_module();

        Logger::logInfo("LocalRouter routing package from host \"%s\" (Module \"%s\"), "
                        "destined for host \"%s\" (Module \"%s\").", sourceHost.c_str(), sourceModule.c_str(), targetHost.c_str(), targetModule.c_str());

        SharedQueue<DataPackage>* inputQueueForRuntime = this->moduleTable.lookupOutputQueue(targetModule);

        if(!inputQueueForRuntime)
        {
            return absl::InvalidArgumentError(absl::StrCat("LocalRouter: Failed to route package to local Module \"", targetModule,"\".\n"
            "Unable to get input queue of the Runtime the Module is running in. Possibly, the Runtime was not registered."));
        }
        if(!inputQueueForRuntime->is_closed())
        {
            inputQueueForRuntime->push_back(dataPackage);
        }
        else
        {
            // Might happen when shutting down or a Runtime has not yet been loaded.
            // Logger::logWarning("LocalRouter inputQueueForRuntime is null, cannot route package. Are we shutting down?");
        }
        return absl::OkStatus();
    }

    absl::Status LocalRouter::routeControlPackage(std::shared_ptr<DataPackage> package)
    {
        Logger::logInfo("LocalRouter routing control package");
        this->moduleTable.controlPackagesQueue().push_front(package);
        return absl::OkStatus();
    }

    bool LocalRouter::canReachHost(const std::string& hostname)
    {
        return hostname == currentHost;
    }
}