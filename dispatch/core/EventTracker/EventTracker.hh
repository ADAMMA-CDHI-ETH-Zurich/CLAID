#pragma once

#include <string>
#include <ctime>

#include "dispatch/proto/claidservice.grpc.pb.h"

using claidservice::PowerProfile;

namespace claid
{
    class EventTracker
    {
        private:
            std::string storageFolderPath;
            std::string getTimeString();

            void logModuleEvent(const std::string& event, const std::string& moduleId, const std::string& moduleType, const std::string extra);
            void logGeneralEvent(const std::string& event);
        public: 
            void onModuleStarted(const std::string& moduleId, const std::string& moduleType);
            void onModuleStopped(const std::string& moduleId, const std::string& moduleType);
            void onModulePaused(const std::string& moduleId, const std::string& moduleType);
            void onModuleResumed(const std::string& moduleId, const std::string& moduleType);
            void onModulePowerProfileApplied(const std::string& moduleId, const std::string& moduleType, PowerProfile powerProfile);
            void onCLAIDStarted();
            void onCLAIDStopped();

            void setStorageFolderPath(const std::string& path);

    };
}