#include "dispatch/core/Router/MasterRouter.hh"
#include "dispatch/core/Router/RoutingTreeParser.hh"
#include "dispatch/core/Logger/Logger.hh"
#include "dispatch/core/Configuration/HostDescription.hh"
#include <sstream>

namespace claid
{
    

    
    absl::Status MasterRouter::buildRoutingTable(const std::string& currentHost, const HostDescriptionMap& hostDescriptions)
    {
        for(const auto& entry : hostDescriptions)
        {
            const HostDescription& host = entry.second;
            const std::string& hostname = host.hostname;  
        
            std::shared_ptr<Router> responsibleRouter = nullptr;
            for(std::shared_ptr<Router> router : this->routers)
            {
                if(router->canReachHost(hostname))
                {
                    if(responsibleRouter != nullptr)
                    {
                        return absl::AlreadyExistsError(absl::StrCat(
                            "MasterRouter failed to build routing table.\n",
                            "Routing for host \"", hostname, "\" is ambiguous, host can be reached by multiple routers."
                        ));
                    }

                    responsibleRouter = router;
                    // Keep looping to check the other routers to detect ambiguities 
                }
            }

            if(responsibleRouter == nullptr)
            {
                return absl::NotFoundError(absl::StrCat(
                    "MasterRouter failed to build routing table.\n",
                    "No Router is able to reach host \"", hostname, "\"."
                ));
            }

            this->routingTable[hostname] = responsibleRouter;
        }

        return absl::OkStatus();
    }


    absl::Status MasterRouter::routePackage(std::shared_ptr<DataPackage> dataPackage) 
    {
        const std::string& sourceHost = dataPackage->source_host();
        const std::string& sourceModule = dataPackage->source_module();

        const std::string& targetHost = dataPackage->target_host();
        const std::string& targetModule = dataPackage->target_module();

        auto it = this->routingTable.find(targetHost);
        if(it == this->routingTable.end())
        {
            // Do what?
            return absl::InvalidArgumentError(absl::StrCat("Host \"", targetHost, "\" is unknown and could not be found in routing tree.\n"));
        }

        Logger::logInfo("MasterRouter received package from host \"%s\" (Module \"%s\"), "
                        "destined for host \"%s\" (Module \"%s\").", sourceHost.c_str(), sourceModule.c_str(), targetHost.c_str(), targetModule.c_str());
                        
        absl::Status status = it->second->routePackage(dataPackage);

        return status;
    }

    void MasterRouter::processQueue()
    {
        while(this->active)
        {
            std::shared_ptr<DataPackage> package;

            package = this->incomingQueue.interruptable_pop_front();

            if(package == nullptr)
            {
                // If the queue is closed, that should mean that MasterRouter::stop was called.
                if(!this->incomingQueue.is_closed())
                {
                    Logger::logWarning("MasterRouter received nullptr");
                }
                continue;
            }

            absl::Status status;
            status = addPackageSourceIfNotSet(package);
            if(!status.ok())
            {
                std::stringstream ss;
                ss << status;
                this->lastError = status;
                Logger::logError("MasterRouter: Failed to route package, got error: %s", ss.str().c_str());
                continue;
            }

            status = addPackageDestinationIfNotSet(package);
            if(!status.ok())
            {
                std::stringstream ss;
                ss << status;
                this->lastError = status;
                Logger::logError("MasterRouter: Failed to route package, got error: %s", ss.str().c_str());
                continue;
            }

            if(!package)
            {
                Logger::logError("MasterRouter: Failed to route package, package is null.");
                continue;
            }

            status = this->routePackage(package);
            if(!status.ok())
            {
                std::stringstream ss;
                ss << status;
                this->lastError = status;
                Logger::logError("MasterRouter: Failed to route package, got error: %s", ss.str().c_str());
                continue;
            }
        }
    }

    absl::Status MasterRouter::start() 
    {
        if(this->active || this->processingThread.get() != nullptr)
        {
            return absl::AlreadyExistsError("MasterRouter: Start was called twice.");
        }

        absl::Status status;
        status = buildRoutingTable(this->currentHost, this->hostDescriptions);
        if(!status.ok())
        {
            return status;
        }


        for(std::shared_ptr<Router> router : this->routers)
        {
            status = router->start();
            if(!status.ok())
            {
                return status;
            }
        }

        this->active = true;
        this->processingThread = make_unique<std::thread>(&MasterRouter::processQueue, this);

        return absl::OkStatus();
    }

    absl::Status MasterRouter::stop()
    {
        Logger::logInfo("MasterRouter::stop called");
        if(!this->active)
        {
            return absl::InvalidArgumentError("Failed to stop MasterRouter. Router is not running.");
        }

        this->active = false;
        this->incomingQueue.interruptOnce();
        this->processingThread->join();
        return absl::OkStatus();
    }   

    absl::Status MasterRouter::stopAfterQueueFinished()
    {
        if(!this->active)
        {
            return absl::InvalidArgumentError("Failed to stop MasterRouter. Router is not running.");
        }

        while(this->incomingQueue.size() > 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        return this->stop();
    }


    bool MasterRouter::canReachHost(const std::string& hostname)
    {
        for(std::shared_ptr<Router> router : this->routers)
        {
            if(router->canReachHost(hostname))
            {
                return true;
            }
        }
        return false;
    }

    absl::Status MasterRouter::getLastError()
    {
        return this->lastError;
    }

    absl::Status MasterRouter::addPackageDestinationIfNotSet(std::shared_ptr<DataPackage> package) const
    {
        // If the target_host is already set, we are done here.
        if(package->target_host() != "")
        {
            return absl::OkStatus();
        }

        // If neither target_host nor target_module are set, there is nothing we can do.
        if(package->target_module() == "")
        {
            return absl::NotFoundError(absl::StrCat(
                "MasterRouter failed to determine destination for package sent by Module \"", package->source_module(), "\" running on host \"", package->source_host(), "\".\n",
                "Neither the the \"target_module\" nor \"target_host\" field has been set for the package. Is there a subscriber for that channel?"
            ));
        }

        const std::string& targetModule = package->target_module();
        std::string targetHost;

        if(!findHostOfModule(targetModule, targetHost))
        {
            return absl::NotFoundError(absl::StrCat(
                "MasterRouter failed to determine destination for package sent by Module \"", package->source_module(), "\" running on host \"", package->source_host(), "\".\n",
                "The package is destined for Module \"", targetModule, "\", however the host of that Module could not be found."
            ));
        }

        package->set_target_host(targetHost);

        return absl::OkStatus();
    }

    absl::Status MasterRouter::addPackageSourceIfNotSet(std::shared_ptr<DataPackage> package) const
    {
        // If the source_host is already set, we are done here.
        if(package->source_host() != "")
        {
            return absl::OkStatus();
        }

        // If neither source_host nor source_module are set, there is nothing we can do.
        if(package->source_module() == "")
        {
            return absl::NotFoundError(absl::StrCat(
                "MasterRouter failed to determine origin of package targeted for Module \"", package->target_module(), "\" running on host \"", package->target_host(), "\".\n",
                "Neither the the \"source_module\" nor \"source_host\" field has been set for the package."
            ));
        }

        const std::string& sourceModule = package->source_module();
        std::string sourceHost;

        if(!findHostOfModule(sourceModule, sourceHost))
        {
            return absl::NotFoundError(absl::StrCat(
                "MasterRouter failed to determine origin of package targeted for Module \"", package->target_module(), "\" running on host \"", package->target_host(), "\".\n",
                "The package is originating from Module \"", sourceModule, "\", however the host of that Module could not be found."
            ));
        }

        package->set_source_host(sourceHost);

        return absl::OkStatus();
    }

    bool MasterRouter::findHostOfModule(const std::string& module, std::string& hostOfModule) const
    {
        for(const auto& moduleEntry : this->moduleDescriptions)
        {
            const std::string& moduleName = moduleEntry.first;
            const ModuleDescription& moduleDescription = moduleEntry.second;
            Logger::logInfo("Module %s %s", moduleName.c_str(), moduleDescription.host.c_str());
            if(moduleName == module)
            {
                hostOfModule = moduleDescription.host;
                return hostOfModule != "";
            }
        }
        return false;
    }
}



    // // Parses the host connection information from the config file into a routing tree.
    // // Afterward, the routing table is built by looking up the currentHost in the tree and mapping the dispatchers accordingly.
    // // Hosts that are connected to the current host either directly or by subsequent servers are being routed by the serverDispatcher.
    // // Hosts that the current host connects to either directly or by intermediate servers are being routed by the clientDispatcher.
    // // Packages within the currentHost itself will be routed by the localDispatcher.

    // absl::Status MasterRouter::buildRoutingTableFromTree(
    //         const std::string& currentHost, const HostDescriptionMap& hostDescriptions)
    // {
    //     RoutingNode* hostNode = this->routingTree.lookupHost(currentHost);

    //     if(hostNode == nullptr)
    //     {
    //         return absl::NotFoundError(absl::StrCat("Error while parsing configuration file. Failed to lookup host \"", currentHost, "\" in the routing tree."));
    //     }

    //     RoutingTree hostTree(hostNode);
    //     std::vector<std::string> childHosts;

    //     hostTree.getChildHostRecursively(childHosts);

    //     for(const auto& entry : hostDescriptions)
    //     {
    //         const HostDescription& host = entry.second;
    //         const std::string& hostname = host.hostname;

    //         if(hostname == currentHost)
    //         {
    //             this->routingTable.insert(make_pair(hostname, this->localRouter));
    //         }
    //         // If the host is "below" us in the routing tree (one of our children or children's children),
    //         // we route it via the ServerDispatcher.
    //         else if(std::find(childHosts.begin(), childHosts.end(), hostname) != childHosts.end())
    //         {
    //             this->routingTable.insert(make_pair(hostname, this->serverRouter));
    //         }
    //         // Else, the host has to be reachable via a host "above" us in the routing tree (a server we connect to either directly or via another server).
    //         // Hence, we route it via the ClientDispatcher.
    //         else
    //         {
    //             this->routingTable.insert(make_pair(hostname, this->clientRouter));
    //         }
    //     }
    //     return absl::OkStatus();
    // }