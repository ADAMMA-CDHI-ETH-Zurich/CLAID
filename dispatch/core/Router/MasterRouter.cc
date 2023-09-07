#include "dispatch/core/Router/MasterRouter.hh"
#include "dispatch/core/Router/RoutingTreeParser.hh"
#include "dispatch/core/Exception/Exception.hh"

namespace claid
{

    MasterRouter(
        SharedQueue<claidservice::DataPackage>& incomingQueue, 
        ModuleTable& moduleTable, 
        ServerTable& serverTable, ClientTable& clienTable) : incomingQueue(incomingQueue)
    {
        this->localRouter = 
            std::static_pointer_cast<Router>(std::make_shared<LocalRouter>(this->localRouterQueue, moduleTable));

        this->serverRouter = 
            std::static_pointer_cast<Router>(std::make_shared<ServerRouter>(this->serverRouterQueue, serverTable));
        
        this->localRouter 
            = std::static_pointer_cast<Router>(std::make_shared<ClientRouter>(this->clientRouterQueue, clienTable));
    }

    // Parses the host connection information from the config file into a routing tree.
    // Afterward, the routing table is built by looking up the currentHost in the tree and mapping the dispatchers accordingly.
    // Hosts that are connected to the current host either directly or by subsequent servers are being routed by the serverDispatcher.
    // Hosts that the current host connects to either directly or by intermediate servers are being routed by the clientDispatcher.
    // Packages within the currentHost itself will be routed by the localDispatcher.
    absl::Status MasterRouter::buildRoutingTable(std::string currentHost, const CLAIDConfig& claidConfig)
    {
        RoutingTreeParser parser;
        RoutingNode* rootNode = parser.parseConfig(claidConfig);

        if(rootNode == nullptr)
        {
            return absl::InvalidArgumentError("Error while parsing configuration file. Tried to parse routing tree from configuration file,\n"
            "but failed to determine root node of the tree.");
        }

        this->routingTree = RoutingTree(rootNode);
        std::string str;
        this->routingTree.toString(str);

        std::cout << str << "\n";

        return this->buildRoutingTableFromTree(claidConfig, currentHost);
    }

    absl::Status MasterRouter::buildRoutingTableFromTree(
            const CLAIDConfig& claidConfig, const std::string& currentHost)
    {
        RoutingNode* hostNode = this->routingTree.lookupHost(currentHost);

        if(hostNode == nullptr)
        {
            return absl::NotFoundError(absl::StrCat("Error while parsing configuration file. Failed to lookup host \"", currentHost, "\" in the routing tree."));
        }

        RoutingTree hostTree(hostNode);
        std::vector<std::string> childHosts;

        hostTree.getChildHostRecursively(childHosts);

        for(size_t i = 0; i < claidConfig.hosts_size(); i++)
        {
            const HostConfig& host = claidConfig.hosts(i);
            const std::string& hostname = host.hostname();

            if(hostname == currentHost)
            {
                this->routingTable.insert(make_pair(hostname, this->localRouterQueue));
            }
            // If the host is "below" us in the routing tree (one of our children or children's children),
            // we route it via the ServerDispatcher.
            else if(std::find(childHosts.begin(), childHosts.end(), hostname) != childHosts.end())
            {
                this->routingTable.insert(make_pair(hostname, this->serverRouterQueue));
            }
            // Else, the host has to be reachable via a host "above" us in the routing tree (a server we connect to either directly or via another server).
            // Hence, we route it via the ClientDispatcher.
            else
            {
                this->routingTable.insert(make_pair(hostname, this->clientRouterQueue));
            }
        }
        return absl::OkStatus();
    }

    void MasterRouter::routePackage(std::shared_ptr<DataPackage> dataPackage) 
    {
        const std::string& target_host_module = dataPackage.target_host_module();

        std::string targetHost;
        std::string targetModule;

        absl::Status status = Router::getTargetHostAndModule(dataPackage, targetHost, targetModule);
        
        if(!status.ok())
        {
            // Do what? 
            // return status;
            printf("Error, failed to parse host from data package failed. Host %s is invalid.\n", dataPackage->target_host_module().c_str())
            return;
        }

        auto it = this->routingTable.find(targetHost);
        if(it == this->routingTable.end())
        {
            // Do what?
            printf("Routing package failed. Host %s is unknown.\n", targetHost.c_str());
            return;
        }

        it->second.push_back(dataPackage);
    }

    void MasterRouter::initialize()
    {
        this->localRouter->start();
        this->serverRouter->start();
        this->clientRouter->start();
    }
}