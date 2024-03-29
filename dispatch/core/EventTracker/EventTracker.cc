/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Stephan Altmüller
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*         http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
***************************************************************************/

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
        Path path = Path::join(this->storageFolderPath, moduleId + "_events.txt");
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
        Logger::logInfo("Logging general event %s %d", storageFolderPath.c_str(), FileUtils::dirExists(storageFolderPath));
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
        this->storageFolderPath = path;
    }

    std::string EventTracker::getStorageFolderPath() const
    {
        return this->storageFolderPath;
    }
}