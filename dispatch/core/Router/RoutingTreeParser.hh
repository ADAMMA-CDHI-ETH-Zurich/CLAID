#pragma once
#include "dispatch/core/CLAIDConfig/CLAIDConfig.hh"
#include "dispatch/core/Router/RoutingNode.hh"
#include "dispatch/core/Router/HostDescription.hh"
#include "dispatch/core/Exception/Exception.hh"

#include <memory>

namespace claid
{
    // Parses a CLAIDConfig into a routing tree.
    class RoutingTreeParser
    {
        private:

            void getHostsAndAddresses(const CLAIDConfig& config, std::vector<HostDescription>& hostDescriptions,
                    std::map<std::string, std::string>& serverHostAddresses )
            {
                hostDescriptions.clear();
                serverHostAddresses.clear();

                for(size_t i = 0; i < config.hosts_size(); i++)
                {
                    const HostConfig& host = config.hosts(i);

                    HostDescription hostDescription;
                    hostDescription.hostname = host.hostname();
                    hostDescription.isServer = host.is_server();
                    hostDescription.hostServerAddress = host.host_server_address();
                    hostDescription.connectTo = host.connect_to();

                    hostDescriptions.push_back(hostDescription);


                    if(hostDescription.isServer)
                    {
                        serverHostAddresses.insert(
                                make_pair(hostDescription.hostServerAddress, 
                                hostDescription.hostname));
                    }

                }
            }

            void createRoutingNodesForEachHost(
                    const std::vector<HostDescription>& hostDescriptions,  
                    std::map<std::string, RoutingNode*>& routingNodes)
            {
                routingNodes.clear();
                for(const HostDescription& host : hostDescriptions)
                {
                    if(routingNodes.find(host.hostname) == routingNodes.end())
                    {
                        RoutingNode* node = new RoutingNode(host.hostname);
                        routingNodes.insert(make_pair(host.hostname, node));
                    }
                    else
                    {
                        CLAID_THROW(claid::Exception, "Error while parsing CLAID configuration to routing tree."
                                    << "Host \"" << host.hostname << "\" was specified more than once.");
                    }
                }
            }

            RoutingNode* combineRoutingNodesToTree(
                    std::map<std::string, RoutingNode*>& routingNodes,
                    const std::vector<HostDescription>& hostDescriptions,
                    const std::map<std::string, std::string>& serverHostAddresses)
            {
                RoutingNode* routingTreeRoot = nullptr;

                for(const HostDescription& host : hostDescriptions)
                {
                    const std::string& connectToAddress = host.connectTo;
                    RoutingNode* hostNode = routingNodes[host.hostname];

                    if(connectToAddress == "")
                    {
                        // This host does not connect to any other node, hence it has to be the root node.

                        if(routingTreeRoot != nullptr)
                        {
                            // Root node was already set, but we found another node that doesnt connect to another host.
                            // Two unconnected nodes -> error.
                            CLAID_THROW(claid::Exception, "Error while parsing CLAID config to routing tree.\n"
                                        << "Host \"" << host.hostname << "\" does not connect to any other host,\n"
                                        << "hence we assume it is the root node of the routing tree.\n"
                                        << "However, host \"" << routingTreeRoot->name << "\" was already set as routing tree root node.\n"
                                        << "Please make sure that every host is connected to another host, except for the initial/root host (i.e., a server that no other server connects to).");
                        }
                        routingTreeRoot = hostNode;
                    }
                    else
                    {
                        auto serverHostAddressesIterator = serverHostAddresses.find(connectToAddress);
                        if(serverHostAddressesIterator == serverHostAddresses.end())
                        {
                            CLAID_THROW(claid::Exception, "Error while parsing CLAID config to routing tree.\n"
                                        << "Host \"" << host.hostname << "\" is configured to connect to a host with address \"" << connectToAddress << "\",\n"
                                        << "however a host with this address was not found.\n"
                                        << "Make sure that that a host with a host_server_address set as \"" << connectToAddress << "\" exists and is a server.");
                        }

                        const std::string& connectToHostname = serverHostAddressesIterator->second; 

                        auto routingNodesIterator = routingNodes.find(connectToHostname);
                        if(routingNodesIterator == routingNodes.end())
                        {
                             CLAID_THROW(claid::Exception, "Error while parsing CLAID config to routing tree.\n"
                                        << "Host \"" << host.hostname << "\" is configured to connect to host \"" << connectToHostname << ",\"\n"
                                        << "however a RoutingNode for that host was not found");
                        }

                        RoutingNode* connectToHostNode = routingNodesIterator->second;

                        // The current host is connecting to another host.
                        // Hence, we add the current host as child to the connectToHost,
                        // and set the parent of the current host to the connectToHost aswell.
                        connectToHostNode->children.push_back(hostNode);
                        hostNode->parent = connectToHostNode;
                    }
                }

                return routingTreeRoot;
            }

        public:
            RoutingNode* parseConfig(const CLAIDConfig& config)
            {
                if(config.hosts_size() == 0)
                {
                    // Empty list, no hosts specified.
                    return nullptr;
                }

                std::vector<HostDescription> hostDescriptions;
                // For each host that is a server, this maps the server's address to it's host name.
                std::map<std::string /* address */, std::string /* host */> serverHostAddresses;
                getHostsAndAddresses(config, hostDescriptions, serverHostAddresses);

                std::map<std::string /* host */, RoutingNode*> routingNodes;
                createRoutingNodesForEachHost(hostDescriptions, routingNodes);

                RoutingNode* routingTreeRoot = 
                    combineRoutingNodesToTree(routingNodes, hostDescriptions, serverHostAddresses);
                
                return routingTreeRoot;
            }
    };
}