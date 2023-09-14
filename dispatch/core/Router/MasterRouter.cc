#include "dispatch/core/Router/MasterRouter.hh"
#include "dispatch/core/Router/RoutingTreeParser.hh"
#include "dispatch/core/Logger/Logger.hh"
#include "dispatch/core/Configuration/HostDescription.hh"
#include <sstream>

namespace claid
{
    MasterRouter::MasterRouter(
                SharedQueue<claidservice::DataPackage>& incomingQueue, 
                std::shared_ptr<LocalRouter> localRouter,
                std::shared_ptr<ClientRouter> clientRouter,
                std::shared_ptr<ServerRouter> serverRouter) : incomingQueue(incomingQueue)
    {
        this->localRouter = 
            std::static_pointer_cast<Router>(localRouter);

        this->clientRouter = 
            std::static_pointer_cast<Router>(clientRouter);
        
        this->serverRouter 
            = std::static_pointer_cast<Router>(serverRouter);
    }

    // Parses the host connection information from the config file into a routing tree.
    // Afterward, the routing table is built by looking up the currentHost in the tree and mapping the dispatchers accordingly.
    // Hosts that are connected to the current host either directly or by subsequent servers are being routed by the serverDispatcher.
    // Hosts that the current host connects to either directly or by intermediate servers are being routed by the clientDispatcher.
    // Packages within the currentHost itself will be routed by the localDispatcher.
    absl::Status MasterRouter::buildRoutingTable(std::string currentHost, const HostDescriptionMap& hostDescriptions)
    {
        RoutingTreeParser parser;
        
        absl::Status status = parser.buildRoutingTree(hostDescriptions, this->routingTree);

        if(!status.ok())
        {
            return status;
        }

        std::string str;
        this->routingTree.toString(str);

        std::cout << str << "\n";

        return this->buildRoutingTableFromTree(currentHost, hostDescriptions);
    }

    absl::Status MasterRouter::buildRoutingTableFromTree(
            const std::string& currentHost, const HostDescriptionMap& hostDescriptions)
    {
        RoutingNode* hostNode = this->routingTree.lookupHost(currentHost);

        if(hostNode == nullptr)
        {
            return absl::NotFoundError(absl::StrCat("Error while parsing configuration file. Failed to lookup host \"", currentHost, "\" in the routing tree."));
        }

        RoutingTree hostTree(hostNode);
        std::vector<std::string> childHosts;

        hostTree.getChildHostRecursively(childHosts);

        for(const auto& entry : hostDescriptions)
        {
            const HostDescription& host = entry.second;
            const std::string& hostname = host.hostname;

            if(hostname == currentHost)
            {
                this->routingTable.insert(make_pair(hostname, this->localRouter));
            }
            // If the host is "below" us in the routing tree (one of our children or children's children),
            // we route it via the ServerDispatcher.
            else if(std::find(childHosts.begin(), childHosts.end(), hostname) != childHosts.end())
            {
                this->routingTable.insert(make_pair(hostname, this->serverRouter));
            }
            // Else, the host has to be reachable via a host "above" us in the routing tree (a server we connect to either directly or via another server).
            // Hence, we route it via the ClientDispatcher.
            else
            {
                this->routingTable.insert(make_pair(hostname, this->clientRouter));
            }
        }
        return absl::OkStatus();
    }

    absl::Status MasterRouter::routePackage(std::shared_ptr<DataPackage> dataPackage) 
    {
        std::string targetHost;
        std::string targetModule;

        absl::Status status = getTargetHostAndModule(*dataPackage.get(), targetHost, targetModule);
        
        if(!status.ok())
        {
            // Do what? 
            // return status;
            return absl::InvalidArgumentError(
                absl::StrCat("Error, failed to parse host from data package failed. Host \"", 
                dataPackage->target_host_module(), "\" is invalid.\n"));
        }

        auto it = this->routingTable.find(targetHost);
        if(it == this->routingTable.end())
        {
            // Do what?
            return absl::InvalidArgumentError(absl::StrCat("Routing package failed. Host \"", targetHost, "\" is unknown.\n"));
        }

        status = it->second->routePackage(dataPackage);

        return status;
    }

    void MasterRouter::processQueue()
    {
        while(this->active)
        {
            std::shared_ptr<DataPackage> package;
            package = this->incomingQueue.pop_front();
            absl::Status status = this->routePackage(package);
            if(!status.ok())
            {
                std::stringstream ss;
                ss << status;
                Logger::printfln("MasterRouter: Failed to route package with error %s", ss.str().c_str());
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
        status = this->localRouter->start();
        if(!status.ok())
        {
            return status;
        }

        status = this->serverRouter->start();
        if(!status.ok())
        {
            return status;
        }

        status = this->clientRouter->start();
        if(!status.ok())
        {
            return status;
        }

        this->active = true;
        this->processingThread = make_unique<std::thread>(&MasterRouter::processQueue, this);

        return absl::OkStatus();
    }
}