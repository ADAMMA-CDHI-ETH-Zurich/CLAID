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

#ifndef CLAID_CONFIG_HH_
#define CLAID_CONFIG_HH_

// #include <algorithm>
// #include <chrono>
// #include <cmath>
// #include <iostream>
#include <memory>
#include <string>
#include "absl/status/status.h"

#include "dispatch/proto/claidservice.pb.h"
#include "dispatch/core/Configuration/HostDescription.hh"
#include "dispatch/core/Configuration/ModuleDescription.hh"
#include "dispatch/core/Configuration/ChannelDescription.hh"
#include "dispatch/core/Logger/LogSinkConfiguration.hh"

using namespace std;

using claidservice::LogMessageSeverityLevel;

namespace claid
{
    class Configuration
    {
        private:
            claidservice::CLAIDConfig config;

            absl::Status loadFileToString(const std::string& path, std::string& content);

        public:

            Configuration();
            Configuration(const claidservice::CLAIDConfig& config);

            absl::Status fromJSONString(const std::string& json);
            absl::Status toJSONString(std::string& jsonOutput) const;

            absl::Status parseFromJSONFile(const std::string& path);

            absl::Status getHostDescriptions(HostDescriptionMap& hostDescriptions) const;
            absl::Status getModuleDescriptions(ModuleDescriptionMap& moduleDescriptions) const;
            absl::Status getChannelDescriptions(ChannelDescriptionMap& channelDescriptions) const;
            absl::Status extractModulesForHost(const string& hostId, const ModuleDescriptionMap& allModuleDescriptions, ModuleDescriptionMap& modulesForHost) const;

            void getLogSinkConfiguration(LogSinkConfiguration& configuration, std::shared_ptr<SharedQueue<LogMessage>> logMessagesQueue) const;

            bool hostExistsInConfiguration(const std::string& hostname) const;

            LogMessageSeverityLevel getMinLogSeverityLevelToPrint(const std::string& hostName) const;

            // bool isDesignerModeEnabled() const;
    };


}  // namespace claid

#endif  // CLAID_CONFIG_H_
