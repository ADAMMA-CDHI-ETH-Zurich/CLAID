#pragma once

#include <string>
#include <map>

namespace claid
{
    struct HostDescription
    {
        std::string hostname;
        bool isServer;
        std::string hostServerAddress;
        std::string connectTo;
    };

    typedef std::map<std::string, HostDescription> HostDescriptionMap;
}