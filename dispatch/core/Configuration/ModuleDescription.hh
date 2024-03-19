#pragma once


#include <map>
#include <string>
#include <vector>

#include "dispatch/core/Configuration/UniqueKeyMap.hh"

namespace claid
{
    struct ModuleDescription
    {
        std::string id;
        std::string moduleClass;
        std::string host;
        std::map<std::string, std::string> inputChannels;
        std::map<std::string, std::string> outputChannels;
        google::protobuf::Struct properties;

        ModuleDescription()
        {

        }

        ModuleDescription(const std::string& id, 
            const std::string& moduleClass, 
            const std::string& host, 
            const google::protobuf::Struct properties) : id(id), moduleClass(moduleClass), host(host), properties(properties)
        {

        }
    };

    typedef UniqueKeyMap<ModuleDescription> ModuleDescriptionMap;
}