#pragma once

#include <string>
#include <memory>
#include <thread>
#include "dispatch/core/CLAID.hh"
class CLAIDWrapper
{
    private:
        claid::CLAID* claidInstance = nullptr;
        std::unique_ptr<std::thread> thread;
    public:
        bool start(const std::string& configPath, const std::string& hostId, const std::string& userId, const std::string& deviceId);
};