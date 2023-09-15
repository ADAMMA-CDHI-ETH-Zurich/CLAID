#pragma once
#include "dispatch/core/Configuration/Configuration.hh"
#include "dispatch/core/Configuration/HostDescription.hh"
#include "dispatch/core/Router/RoutingNode.hh"

#include <memory>

namespace claid
{
    // Parses a CLAIDConfig into a routing tree.
    class RoutingTreeParser
    {
        private:

            absl::Status checkUniqueAddresses(const HostDescriptionMap& hostDescriptions) const
            {
                
                for(const auto& entry : hostDescriptions)
                {
                    const HostDescription& host = entry.second;

                    if(host.isServer)
                    {
                        // Check if a server with this address already exists.
                        if(hostDescriptions.find(host.hostServerAddress) != hostDescriptions.end())
                        {
                            return absl::AlreadyExistsError(
                                absl::StrCat(
                                    "Multiple hosts were configured to be a server with address \"",
                                    host.hostServerAddress, "\". Each host that is a server needs to have a unique address."));
                        }
                    }
                }

                return absl::OkStatus();
            }

            absl::Status createRoutingNodesForEachHost(
                    const HostDescriptionMap& hostDescriptions,  
                    std::map<std::string, RoutingNode*>& routingNodes)
            {
                routingNodes.clear();
                for(const auto& entry : hostDescriptions)
                {
                    const HostDescription& host = entry.second;

                    if(routingNodes.find(host.hostname) == routingNodes.end())
                    {
                        RoutingNode* node = new RoutingNode(host.hostname);
                        routingNodes.insert(make_pair(host.hostname, node));
                    }
                    else
                    {
                        return absl::AlreadyExistsError(
                            absl::StrCat("Error while parsing CLAID configuration to routing tree.",
                                "Host \"", host.hostname, "\" was specified more than once."));
                    }
                }
                return absl::OkStatus();
            }

            absl::Status combineRoutingNodesToTree(
                    std::map<std::string, RoutingNode*>& routingNodes,
                    const HostDescriptionMap& hostDescriptions,
                    RoutingNode*& routingTreeRoot)
            {
                routingTreeRoot = nullptr;

                for(const auto& entry : hostDescriptions)
                {
                    const HostDescription& host = entry.second;
                    const std::string& connectToHostname = host.connectTo;
                    RoutingNode* hostNode = routingNodes[host.hostname];

                    if(host.isServer && host.hostServerAddress.empty())
                    {
                        return absl::NotFoundError(absl::StrCat("Host \"", host.hostname, "\" was configured to be a server, ",
                            "however, no hostServerAddress was specified."));
                    }

                    if(connectToHostname == "")
                    {
                        // This host does not connect to any other node, hence it has to be the root node.

                        if(routingTreeRoot != nullptr)
                        {
                            // Root node was already set, but we found another node that doesnt connect to another host.
                            // Two unconnected nodes -> error.
                            return absl::InvalidArgumentError(absl::StrCat("Error while building routing tree.\n",
                                        "Host \"", host.hostname, "\" does not connect to any other host,\n",
                                        "hence we assume it is the root node of the routing tree.\n",
                                        "However, host \"", routingTreeRoot->name, "\" was already set as routing tree root node.\n",
                                        "Please make sure that every host is connected to another host, except for the initial/root host (i.e., a server that no other server connects to)."));
                        }
                        routingTreeRoot = hostNode;
                    }
                    else
                    {
                        auto hostIterator = hostDescriptions.find(connectToHostname);
                        if(hostIterator == hostDescriptions.end())
                        {
                            return absl::NotFoundError(absl::StrCat("Error while building routing tree.\n",
                                        "Host \"", host.hostname, "\" is configured to connect to a host with address \"", connectToHostname, "\",\n",
                                        "however a host with this address was not found.\n",
                                        "Make sure that that a host with a host_server_address set as \"", connectToHostname, "\" exists and is a server."));
                        }

                        const HostDescription& connectToHostDescription = hostIterator->second;

                        if(!connectToHostDescription.isServer)
                        {
                            return absl::NotFoundError(absl::StrCat("Error while building routing tree.\n",
                                        "Host \"", host.hostname, "\" is configured to connect to a host with address \"", connectToHostname, "\",\n",
                                        "However host\"", connectToHostname, "\" is not configured as server."));
                        }


                        auto routingNodesIterator = routingNodes.find(connectToHostname);
                        if(routingNodesIterator == routingNodes.end())
                        {
                            return absl::InvalidArgumentError(absl::StrCat("Error while building routing tree.\n",
                                        "Host \"", host.hostname, "\" is configured to connect to host \"", connectToHostname, ",\"\n",
                                        "however a RoutingNode for that host was not found"));
                        }

                        RoutingNode* connectToHostNode = routingNodesIterator->second;

                        // The current host is connecting to another host.
                        // Hence, we add the current host as child to the connectToHost,
                        // and set the parent of the current host to the connectToHost aswell.
                        connectToHostNode->children.push_back(hostNode);
                        hostNode->parent = connectToHostNode;
                    }
                }

                return absl::OkStatus();
            }

        public:
            absl::Status buildRoutingTree(const HostDescriptionMap& hostDescriptions, RoutingTree& routingTree)
            {
                if(hostDescriptions.empty())
                {
                    return absl::InvalidArgumentError(
                        absl::StrCat("RoutingTreeParser: Failed to build routing tree, because no hosts were specified.",
                        ("Host description map is empty, i.e., no hosts were specified.")));
                }

                absl::Status status = checkUniqueAddresses(hostDescriptions);
                if(!status.ok())
                {
                    return status;
                }

                std::map<std::string /* host */, RoutingNode*> routingNodes;
                status = createRoutingNodesForEachHost(hostDescriptions, routingNodes);

                if(!status.ok()) { 
                    return status; 
                }

                RoutingNode* routingTreeRoot = nullptr;
                status = 
                    combineRoutingNodesToTree(routingNodes, hostDescriptions, routingTreeRoot);
                                
                
                if(!status.ok())
                {
                    return status;
                }

                routingTree = RoutingTree(routingTreeRoot);
                return absl::OkStatus();
            }
    };
}