#include "dispatch/core/ContinuityTracker/ContinuityTracker.hpp"
#include "dispatch/core/Utilities/Time.hh"
#include "dispatch/core/Utilities/Path.hh"
#include <sstream>
#include <fstream>

namespace claid
{
    std::string EventTracker::getTimeString()
    {
        Time time = Time::now();
        return time.strftime("%y.%m.%d %H:%M:%S");
    }

    void EventTracker::logModuleEvent(const std::string& event, const std::string& moduleId, const std::string& moduleType)
    {   
        if(storageFolderPath == "")
        {
            return;
        }
        Path path = Path::join(this->storageFolderPath, moduleId, "_events.txt");
        std::ofstream file(path.toString(), std::ios::app);

        if(!file.is_open())
        {
            Logger::logError("EventTracker failed to open file \"%s\". Cannot store event \"%s\" for Module \"%s\" (%s)", 
                path.to_string().c_str(), event.to_string().c_str(), moduleId.c_str(), moduleType.c_str());
            return;
        }
        Time time = Time::now();
        std::string timeString = time.strftime("%y.%m.%d %H:%M:%S");;
        file << timeString << ", ", event << ", " << moduleId << ", " << moduleType << ", " << Time::now().toUnixTimestampMilliseconds() << "\n";
    }

    void EventTracker::logGeneralEvent(const std::string& event)
    {   
        if(storageFolderPath == "")
        {
            return;
        }

        Path path = Path::join(this->storageFolderPath, "CLAID_events.txt");
        std::ofstream file(path.toString(), std::ios::app);

        if(!file.is_open())
        {
            Logger::logError("EventTracker failed to open file \"%s\". Cannot store event \"%s\".", 
                path.to_string().c_str(), event.to_string().c_str());
            return;
        }
        Time time = Time::now();
        std::string timeString = time.strftime("%y.%m.%d %H:%M:%S");;
        file << timeString << ", ", event << ", " << Time::now().toUnixTimestampMilliseconds() << "\n";
    }

    void EventTracker::onModuleStarted(const std::string& moduleId, const std::string& moduleType)
    {
        logModuleEvent("OnModuleStarted", moduleId, moduleType);
    }

    void EventTracker::onModuleStopped(const std::string& moduleId, const std::string& moduleType)
    {   
        logModuleEvent("OnModuleStopped", moduleId, moduleType);
    }

    void EventTracker::onModulePaused(const std::string& moduleId, const std::string& moduleType)
    {   
        logModuleEvent("OnModulePaused", moduleId, moduleType);
    }

    void EventTracker::onModuleResumed(const std::string& moduleId, const std::string& moduleType)
    {   
        logModuleEvent("OnModuleResumed", moduleId, moduleType);
    }

    void EventTracker::onCLAIDStarted()
    {
        logGeneralEvent("OnCLAIDStarted");
    }

    void EventTracker::onCLAIDStopped()
    {
        logGeneralEvent("OnCLAIDStopped");
    }
}