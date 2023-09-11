#pragma once

#include "dispatch/core/Configuration/Configuration.hh"
#include "dispatch/core/Router/MasterRouter.hh"

namespace claid
{
    absl::Status populateModuleTable(
        const ModuleDescriptionMap& moduleDescriptions,
        const ChannelDescriptionMap& channelDescriptions, 
        claid::ModuleTable& moduleTable);

    absl::Status start(const std::string& configurationPath, const std::string& currentHost);
}