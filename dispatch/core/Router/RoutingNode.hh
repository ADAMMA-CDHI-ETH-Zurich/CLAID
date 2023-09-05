#pragma once

#include <string>
#include <vector>

namespace claid
{
    struct RoutingNode
    {
        std::string name;
        
        RoutingNode* parent = nullptr;
        std::vector<RoutingNode*> children;


        RoutingNode()
        {

        }

        RoutingNode(const std::string& name) : name(name)
        {

        }
    };
}   
