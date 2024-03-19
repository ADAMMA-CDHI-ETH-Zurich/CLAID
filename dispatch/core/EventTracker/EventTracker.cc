#include "dispatch/core/EventTracker/EventTracker.hh"
#include "dispatch/core/Utilities/Time.hh"
#include "dispatch/core/Utilities/Path.hh"
#include "dispatch/core/Utilities/FileUtils.hh"
#include "dispatch/core/Utilities/StringUtils.hh"
#include "dispatch/core/proto_util.hh"
#include <sstream>
#include <fstream>

namespace claid
{
    std::string EventTracker::getTimeString()
    {
        Time time = Time::now();
        return time.strftime("%d.%m.%y %H:%M:%S");
    }

    void EventTracker::logModuleEvent(const std::string& event, const std::string& moduleId, const std::string& moduleType, std::string extra = "")
    {   
        if(storageFolderPath == "" || !FileUtils::dirExists(storageFolderPath))
        {
            return;
        }
        Path path = Path::join(this->storageFolderPath, moduleId, "_events.txt");
        std::ofstream file(path.toString(), std::ios::app);

        if(!file.is_open())
        {
            Logger::logError("EventTracker failed to open file \"%s\". Cannot store event \"%s\" for Module \"%s\" (%s)", 
                path.toString().c_str(), event.c_str(), moduleId.c_str(), moduleType.c_str());
            return;
        }
        Time time = Time::now();
        std::string timeString = time.strftime("%y.%m.%d %H:%M:%S");;

        if(extra != "")
        {
            file << timeString << ", " << time.toUnixTimestampMilliseconds() << ", " << event << ", " << moduleId << ", " << moduleType << ", " << extra << "\n";
        }
        else
        {
            file << timeString << ", " << time.toUnixTimestampMilliseconds() << ", " << event << ", " << moduleId << ", " << moduleType << "\n";
        }
    }

    void EventTracker::logGeneralEvent(const std::string& event)
    {   
        if(storageFolderPath == "" || !FileUtils::dirExists(storageFolderPath))
        {
            return;
        }

        Path path = Path::join(this->storageFolderPath, "CLAID_events.txt");
        std::ofstream file(path.toString(), std::ios::app);

        if(!file.is_open())
        {
            Logger::logError("EventTracker failed to open file \"%s\". Cannot store event \"%s\".", 
                path.toString().c_str(), event.c_str());
            return;
        }
        Time time = Time::now();
        std::string timeString = time.strftime("%y.%m.%d %H:%M:%S");;
        file << timeString << ", " << time.toUnixTimestampMilliseconds() << ", " << event << "\n";
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

    void EventTracker::onModulePowerProfileApplied(const std::string& moduleId, const std::string& moduleType, PowerProfile powerProfile)
    {
        std::string powerProfileInfo = messageToString(powerProfile);
        StringUtils::stringReplaceAll(powerProfileInfo, "\n", "");
        logModuleEvent("OnModulePowerProfileApplied", moduleId, moduleType, powerProfileInfo);
    }

    void EventTracker::onCLAIDStarted()
    {
        logGeneralEvent("OnCLAIDStarted");
    }

    void EventTracker::onCLAIDStopped()
    {
        logGeneralEvent("OnCLAIDStopped");
    }

    void EventTracker::setStorageFolderPath(const std::string& path)
    {
        if(!FileUtils::dirExists(path))
        {
            if(!FileUtils::createDirectoriesRecursively(path))
            {
                Logger::logError("Failed to setup EventTracker, cannot create path \"%s\"", path.c_str());
                return;
            }
        }
    }
}