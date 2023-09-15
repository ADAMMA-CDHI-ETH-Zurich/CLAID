#pragma once

#include <string>
#include <map>
#include "dispatch/core/Configuration/UniqueKeyMap.hh"

namespace claid
{
    struct HostDescription
    {
        std::string hostname;
        bool isServer;
        std::string hostServerAddress;
        std::string connectTo;

        HostDescription()
        {

        }

        HostDescription(const std::string& hostname, 
            bool isServer, 
            const std::string& hostServerAddress, 
            const std::string& connectTo) : hostname(hostname), isServer(isServer), 
                                            hostServerAddress(hostServerAddress), connectTo(connectTo)
        {

        }
    };

    typedef UniqueKeyMap<HostDescription> HostDescriptionMap;

}