#pragma once

#include "dispatch/core/Router/RoutingNode.hh"
#include <stack>

namespace claid
{
    class RoutingTree
    {
        private:
            RoutingNode* rootNode;

        public:
            RoutingTree()
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

            void toString(std::string& output)
            {
                output.clear();
                
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