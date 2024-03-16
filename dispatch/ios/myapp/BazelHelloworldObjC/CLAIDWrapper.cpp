#include "CLAIDWrapper.hpp"

#include "dispatch/core/CLAID.hh"
bool CLAIDWrapper::start(const std::string& configPath, const std::string& hostId, const std::string& userId, const std::string& deviceId)
{
    if(claidHandle != nullptr)
    {
        return false;
    }

    claid::CLAID* claidInstance = new claid::CLAID();

    bool result = claidInstance->start("localhost:1337", configPath, hostId, userId, deviceId);
    return result;
}


