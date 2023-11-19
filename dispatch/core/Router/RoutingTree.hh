#pragma once

#include "dispatch/core/Router/RoutingNode.hh"
#include "dispatch/core/Router/RoutingDirection.hh"
#include <stack>

namespace claid
{
    class RoutingTree
    {
        private:
            RoutingNode* rootNode = nullptr;

        public:
            RoutingTree() : rootNode(nullptr)
            {

            }

            RoutingTree(RoutingNode* rootNode) : rootNode(rootNode)
            {

            }

            RoutingNode* lookupHost(const std::string& hostname) const
            {
                if(this->rootNode == nullptr)
                {
                    return nullptr;
                }


                std::stack<RoutingNode*> nodesToCheck;
                nodesToCheck.push(this->rootNode);

                RoutingNode* currentNode;

                while(nodesToCheck.size() != 0)
                {
                    currentNode = nodesToCheck.top();
                    nodesToCheck.pop();

                    if(currentNode->name == hostname)
                    {
                        return currentNode;
                    }

                    for(RoutingNode* child : currentNode->children)
                    {
                        nodesToCheck.push(child);
                    }
                }

                return nullptr;
            }

            // Traverses the tree from the current host and returns the name
            // of all children and the children of the children (etc.).
            void getChildHostRecursively(std::vector<std::string>& childHosts) const
            {
                if(this->rootNode == nullptr)
                {
                    return;
                }
                childHosts.clear();

                std::stack<const RoutingNode*> nodesStack;
                for(const RoutingNode* child : this->rootNode->children)
                {
                    nodesStack.push(child);
                }

                const RoutingNode* currentNode;
                while(nodesStack.size() != 0)
                {
                    currentNode = nodesStack.top();
                    nodesStack.pop();

                    childHosts.push_back(currentNode->name);

                    for(const RoutingNode* child : currentNode->children)
                    {
                        nodesStack.push(child);
                    }
                }

            }

            bool getRouteFromHostToHost(const std::string& sourceHost, 
                                    const std::string& targetHost, 
                                    std::vector<std::string>& hostToHostPath,
                                    RoutingDirection& routingDirection) const
            {
                hostToHostPath.clear();
                if(sourceHost == targetHost)
                {
                    hostToHostPath.push_back(targetHost);
                    routingDirection = RoutingDirection::ROUTE_LOCAL;
                    return true;
                }

                RoutingNode* sourceHostNode = this->lookupHost(sourceHost);
                RoutingNode* currentNode = sourceHostNode;

                bool routingUpwardsRequired = false;

                while(currentNode != nullptr)
                {
                    std::vector<std::string> childHosts;
                    RoutingTree subTree(currentNode);
                    subTree.getChildHostRecursively(childHosts);

                    // Can we reach the targetHost from the current host?
                    // Is the target host one of our childs (or child of a child ?)
                    if(std::find(childHosts.begin(), childHosts.end(), targetHost) != childHosts.end())
                    {
                        // If yes, we just have to go upwards from the childNode until we reach the current host,
                        // and add the intermediate hosts in reverse order.
    
                        RoutingNode* intermediateNode = this->lookupHost(targetHost);
                        std::vector<std::string> intermediateHosts;
                        while(intermediateNode != currentNode)
                        {
                            intermediateHosts.push_back(intermediateNode->name);
                            intermediateNode = intermediateNode->parent;
                            if(intermediateNode == nullptr)
                            {
                                // Failed to reach currentNode..
                                return false;
                            }
                        }
                        // Add intermediate hosts in reverse order.
                        hostToHostPath.insert( hostToHostPath.end(), intermediateHosts.rbegin(), intermediateHosts.rend() );

                        // If the targetHost is a direct child of the sourceHostNode (i.e., first iteration of the while loop),
                        // this means that means that the sourceHost is a server and the targetHost is connected to that server
                        // either directly or indirectly. In that case, the package has to be routed downards in the routing tree.
                        routingDirection = routingUpwardsRequired ? RoutingDirection::ROUTE_UP : RoutingDirection::ROUTE_DOWN;
                        return true;
                    }
                    else
                    {
                        // Go up in the tree and try again.
                        currentNode = currentNode->parent;

                        // Store the new currentNode in the hostToHostPath.
                        // That means for the sourceHost to reach the targetHost, it would
                        // have to send a package "upward" in the tree, 
                        hostToHostPath.push_back(currentNode->name);
                        routingUpwardsRequired = true;
                    }
                }

                // currentNode is nullptr, reached top of the tree but did not find host.
                return false;
            }

            void toString(std::string& output) const
            {
                output.clear();
                if(this->rootNode == nullptr)
                {
                    return;
                }
                
                // Int of the pair is the "intendation" (i.e., level in the tree).
                std::stack<std::pair<int, RoutingNode*>> nodesToCheck;
                nodesToCheck.push(std::make_pair(0, this->rootNode));

                std::pair<int, RoutingNode*> currentEntry;

                while(nodesToCheck.size() != 0)
                {
                    currentEntry = nodesToCheck.top();
                    nodesToCheck.pop();

                    const int level = currentEntry.first;
                    RoutingNode* currentNode = currentEntry.second;

                    for(int i = 0; i < level; i++)
                    {
                        output += "\t";
                    }
                    output += currentNode->name;
                    output += "\n";

                    for(RoutingNode* child : currentNode->children)
                    {
                        nodesToCheck.push(std::make_pair(level + 1, child));
                    }
                }
            }

            bool isDirectChildOfHost(const std::string& suggestedChild, const std::string& parent)
            {
                RoutingNode* parentNode = lookupHost(parent);
                if(parentNode == nullptr)
                {
                    return false;
                }

                std::vector<std::string> childNames;
                for(RoutingNode* child : parentNode->children)
                {
                    childNames.push_back(child->name);
                }

                auto it = std::find(childNames.begin(), childNames.end(), suggestedChild);

                return it != childNames.end();
            }
    };
}