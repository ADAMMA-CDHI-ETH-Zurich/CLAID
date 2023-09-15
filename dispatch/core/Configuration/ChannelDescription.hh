#pragma once

#include <map>
#include <vector>
#include <string>

#include "dispatch/core/Configuration/UniqueKeyMap.hh"

namespace claid
{
    struct ChannelDescription
    {
        std::string channelName;
        std::vector<std::string> publisherModules;
        std::vector<std::string> subscriberModules;
    };

    typedef UniqueKeyMap<ChannelDescription> ChannelDescriptionMap;
}