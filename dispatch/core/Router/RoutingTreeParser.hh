/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Stephan Altmüller
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*         http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
***************************************************************************/

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

                    if(host.isServer())
                    {
                        // Check if a server with this address already exists.
                        if(hostDescriptions.find(host.getHostServerAddress()) != hostDescriptions.end())
                        {
                            return absl::AlreadyExistsError(
                                absl::StrCat(
                                    "Multiple hosts were configured to be a server with address \"",
                                    host.getHostServerAddress(), "\". Each host that is a server needs to have a unique address."));
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

                    if(routingNodes.find(host.getHostname()) == routingNodes.end())
                    {
                        RoutingNode* node = new RoutingNode(host.getHostname());
                        routingNodes.insert(make_pair(host.getHostname(), node));
                    }
                    else
                    {
                        return absl::AlreadyExistsError(
                            absl::StrCat("Error while parsing CLAID configuration to routing tree.",
                                "Host \"", host.getHostname(), "\" was specified more than once."));
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
                    const std::string& connectToHostname = host.getConnectToAddress();
                    RoutingNode* hostNode = routingNodes[host.getHostname()];

                    if(host.isServer() && host.getHostServerAddress().empty())
                    {
                        return absl::NotFoundError(absl::StrCat("Host \"", host.getHostname(), "\" was configured to be a server, ",
                            "however, no hostServerAddress was specified."));
                    }

                    if(host.getConnectToAddress() == host.getHostname())
                    {
                        return absl::InvalidArgumentError(
                            absl::StrCat("Host \"", host.getHostname(), "\" was configured to connect to itself. Loops are not allowed in the routing graph!"));
                    }

                    if(connectToHostname == "")
                    {
                        // This host does not connect to any other node, hence it has to be the root node.

                        if(routingTreeRoot != nullptr)
                        {
                            // Root node was already set, but we found another node that doesnt connect to another host.
                            // Two unconnected nodes -> error.
                            return absl::InvalidArgumentError(absl::StrCat("Error while building routing tree.\n",
                                        "Host \"", host.getHostname(), "\" does not connect to any other host,\n",
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
                                        "Host \"", host.getHostname(), "\" is configured to connect to a host with address \"", connectToHostname, "\",\n",
                                        "however a host with this address was not found.\n",
                                        "Make sure that that a host with a host_server_address set as \"", connectToHostname, "\" exists and is a server."));
                        }

                        const HostDescription& connectToHostDescription = hostIterator->second;

                        if(!connectToHostDescription.isServer())
                        {
                            return absl::NotFoundError(absl::StrCat("Error while building routing tree.\n",
                                        "Host \"", host.getHostname(), "\" is configured to connect to a host with address \"", connectToHostname, "\",\n",
                                        "However host\"", connectToHostname, "\" is not configured as server."));
                        }


                        auto routingNodesIterator = routingNodes.find(connectToHostname);
                        if(routingNodesIterator == routingNodes.end())
                        {
                            return absl::InvalidArgumentError(absl::StrCat("Error while building routing tree.\n",
                                        "Host \"", host.getHostname(), "\" is configured to connect to host \"", connectToHostname, ",\"\n",
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