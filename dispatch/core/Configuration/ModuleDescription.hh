#pragma once


#include <map>
#include <string>
#include <vector>

namespace claid
{
    struct ModuleDescription
    {
        std::string id;
        std::string moduleClass;
        std::vector<std::string> inputChannels;
        std::vector<std::string> outputChannels;
        std::map<std::string, std::string> properties;

        ModuleDescription()
        {

        }

        ModuleDescription(const std::string& id, 
        const std::string& moduleClass, 
        const std::map<std::string, std::string> properties) : id(id), moduleClass(moduleClass), properties(properties)
        {

        }
    };

    typedef std::map<std::string, ModuleDescription> ModuleDescriptionMap;
}