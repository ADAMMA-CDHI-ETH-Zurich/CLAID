#include "dispatch/core/Router/Router.hh"
#include "dispatch/core/Router/RoutingTreeParser.hh"
#include "dispatch/core/Exception/Exception.hh"

namespace claid
{
    // Parses the host connection information from the config file into a routing tree.
    // Afterward, the routing table is built by looking up the currentHost in the tree and mapping the dispatchers accordingly.
    // Hosts that are connected to the current host either directly or by subsequent servers are being routed by the serverDispatcher.
    // Hosts that the current host connects to either directly or by intermediate servers are being routed by the clientDispatcher.
    // Packages within the currentHost itself will be routed by the localDispatcher.
    void Router::buildRoutingTable(std::string currentHost, const CLAIDConfig& claidConfig)
    {
        RoutingTreeParser parser;
        RoutingNode* rootNode = parser.parseConfig(claidConfig);

        if(rootNode == nullptr)
        {
            CLAID_THROW(claid::Exception, "Error while parsing configuration file. Tried to parse routing tree from configuration file,\n"
            << "but failed to determine root node of the tree.");
        }

        this->routingTree = RoutingTree(rootNode);
        std::string str;
        this->routingTree.toString(str);

        std::cout << str << "\n";

        return;
        this->buildRoutingTableFromTree(claidConfig, currentHost);
    }

    void Router::buildRoutingTableFromTree(
            const CLAIDConfig& claidConfig, const std::string& currentHost)
    {
        RoutingNode* hostNode = this->routingTree.lookupHost(currentHost);

        if(hostNode == nullptr)
        {
            CLAID_THROW(claid::Exception, "Error while parsing configuration file. Failed to lookup host \"" << currentHost << "\" in the routing tree.");
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
                this->routingTable.insert(make_pair(hostname, this->localDispatcher));
            }
            // If the host is "below" us in the routing tree (one of our children or children's children),
            // we route it via the ServerDispatcher.
            else if(std::find(childHosts.begin(), childHosts.end(), hostname) != childHosts.end())
            {
                this->routingTable.insert(make_pair(hostname, this->serverDispatcher));
            }
            // Else, the host has to be "above" us in the routing tree (a server we connect to either directly or via another server).
            // Hence, we route it via the ClientDispatcher.
            else
            {
                this->routingTable.insert(make_pair(hostname, this->clientDispatcher));
            }
        }
    }

    bool Router::routePackage(const DataPackage& dataPackage)
    {
        return false;
    }
}