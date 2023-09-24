#pragma once

#include "dispatch/core/Router/RoutingNode.hh"
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

            RoutingNode* lookupHost(const std::string& hostname)
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

            bool getPathFromHostToHost(const std::string& sourceHost, const std::string& targetHost, std::vector<std::string>& hostToHostPath)
            {
                hostToHostPath.clear();
                if(sourceHost == targetHost)
                {
                    hostToHostPath.push_back(targetHost);
                    return true;
                }

                RoutingNode* currentNode = this->lookupHost(sourceHost);

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
                    }
                }

                // currentNode is nullptr, reached top of the tree but did not find host.
                return false;
            }

            void toString(std::string& output)
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
    };
}