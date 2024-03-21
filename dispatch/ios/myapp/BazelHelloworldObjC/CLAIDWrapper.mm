#include "CLAIDWrapper.hpp"
#import <Foundation/Foundation.h>
#include "dispatch/core/CLAID.hh"
bool CLAIDWrapper::start(const std::string& configPath, const std::string& hostId, const std::string& userId, const std::string& deviceId)
{
    if(claidInstance != nullptr)
    {
        return false;
    }

    claidInstance = new claid::CLAID();
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    std::string stdDocuments([documentsDirectory UTF8String]);

    thread = std::make_unique<std::thread>([=, this] {
        bool result = claidInstance->start("localhost:1337", configPath, hostId, userId, deviceId, stdDocuments);
    });
    return true;
}


