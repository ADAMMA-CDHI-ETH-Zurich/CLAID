#pragma once

#include <map>
#include <vector>
#include <string>

namespace claid
{
    struct ChannelDescription
    {
        std::string channelName;
        std::vector<std::string> publisherModules;
        std::vector<std::string> subscriberModules;
    };

    typedef std::map<std::string, ChannelDescription> ChannelDescriptionMap;
}