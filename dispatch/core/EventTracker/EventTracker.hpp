#pragma once

#include <string>
#include <ctime>

namespace claid
{
    class EventTracker
    {
        private:
            std::string storageFolderPath;
            std::string getTimeString();

            void logModuleEvent(const std::string& event, const std::string& moduleId, const std::string& moduleType);
            void logGeneralEvent(const std::string& event);
        public: 
            void onModuleStarted(const std::string& moduleId, const std::string& moduleType);
            void onModuleStopped(const std::string& moduleId, const std::string& moduleType);
            void onModulePaused(const std::string& moduleId, const std::string& moduleType);
            void onModuleResumed(const std::string& moduleId, const std::string& moduleType);
            void onCLAIDStarted();
            void onCLAIDStopped();
    };
}