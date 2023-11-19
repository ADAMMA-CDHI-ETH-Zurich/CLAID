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
        std::string targetHost = dataPackage->target_host();
        std::string targetModule = dataPackage->target_module();

        auto it = this->routingTable.find(targetHost);
        if(it == this->routingTable.end())
        {
            // Do what?
            return absl::InvalidArgumentError(absl::StrCat("Host \"", targetHost, "\" is unknown and could not be found in routing tree.\n"));
        }

        absl::Status status = it->second->routePackage(dataPackage);

        return status;
    }

    void MasterRouter::processQueue()
    {
        while(this->active)
        {
            std::shared_ptr<DataPackage> package;

            package = this->incomingQueue.pop_front();

            if(!package)
            {
                Logger::logError("MasterRouter: Failed to route package, package is null.");
                return;
            }

            absl::Status status = this->routePackage(package);
            if(!status.ok())
            {
                std::stringstream ss;
                ss << status;
                Logger::logError("MasterRouter: Failed to route package with error %s", ss.str().c_str());
                return;
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