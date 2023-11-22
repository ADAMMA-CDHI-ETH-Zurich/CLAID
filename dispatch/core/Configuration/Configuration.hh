#ifndef CLAID_CONFIG_HH_
#define CLAID_CONFIG_HH_

// #include <algorithm>
// #include <chrono>
// #include <cmath>
// #include <iostream>
#include <memory>
#include <string>
#include "absl/status/status.h"

#include "dispatch/proto/claidconfig.pb.h"
#include "dispatch/core/Configuration/HostDescription.hh"
#include "dispatch/core/Configuration/ModuleDescription.hh"
#include "dispatch/core/Configuration/ChannelDescription.hh"

using namespace std;


namespace claid
{
    class Configuration
    {
        private:
            claidservice::CLAIDConfig config;

            absl::Status loadFileToString(const std::string& path, std::string& content);

        public:

            absl::Status fromJSONString(const std::string& json);
            absl::Status toJSONString(std::string& jsonOutput) const;

            absl::Status parseFromJSONFile(const std::string& path);

            absl::Status getHostDescriptions(HostDescriptionMap& hostDescriptions) const;
            absl::Status getModuleDescriptions(ModuleDescriptionMap& moduleDescriptions) const;
            absl::Status getChannelDescriptions(ChannelDescriptionMap& channelDescriptions) const;
            absl::Status extractModulesForHost(const string& hostId, const ModuleDescriptionMap& allModuleDescriptions, ModuleDescriptionMap& modulesForHost) const;

            bool hostExistsInConfiguration(const std::string& hostname) const;
    };


}  // namespace claid

#endif  // CLAID_CONFIG_H_
