#pragma once

#include <string>

namespace claid
{
    struct HostDescription
    {
        std::string hostname;
        bool isServer;
        std::string hostServerAddress;
        std::string connectTo;
    };
}