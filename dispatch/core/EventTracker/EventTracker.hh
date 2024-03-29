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
            std::string getStorageFolderPath() const;

    };
}