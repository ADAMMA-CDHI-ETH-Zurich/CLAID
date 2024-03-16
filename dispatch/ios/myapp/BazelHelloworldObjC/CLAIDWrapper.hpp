#pragma once

#include <string>

class CLAIDWrapper
{
    private:
        void* claidHandle = nullptr; 

    public:
        bool start(const std::string& configPath, const std::string& hostId, const std::string& userId, const std::string& deviceId);
};