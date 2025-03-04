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

#include "dispatch/core/Configuration/Configuration.hh"
#include "dispatch/core/Logger/Logger.hh"

#include "google/protobuf/json/json.h"

#include <fstream>

using claidservice::HostConfig;
using claidservice::ModuleConfig;

namespace claid
{
    Configuration::Configuration()
    {

    }

    Configuration::Configuration(const claidservice::CLAIDConfig& config) : config(config)
    {

    }

    absl::Status Configuration::fromJSONString(const std::string& json)
    {
        google::protobuf::json::ParseOptions options2;
        absl::Status status = JsonStringToMessage(json, &this->config, options2);
        Logger::logInfo("status %s", status.ToString().c_str());
        return status;
    }

    absl::Status Configuration::toJSONString(std::string& jsonOutput) const
    {
        jsonOutput = "";
        google::protobuf::json::PrintOptions options;
        options.add_whitespace = true;
        options.always_print_fields_with_no_presence = true;
        options.preserve_proto_field_names = true;
        absl::Status status = MessageToJsonString(this->config, &jsonOutput, options);
        return status;
    }

    absl::Status Configuration::parseFromJSONFile(const std::string& filePath)
    {
        std::string jsonContent;
        absl::Status status = this->loadFileToString(filePath, jsonContent);
        if(!status.ok())
        {
            return status;
        }
        Logger::logInfo("Loaded file from string %s\n", jsonContent.c_str());

        return this->fromJSONString(jsonContent);
    }

    absl::Status Configuration::loadFileToString(const std::string& filePath, std::string& buffer)
    {
        std::ifstream file(filePath);

        if(!file.is_open())
        {
            return absl::InvalidArgumentError(absl::StrCat("Configuration: Could not open file \"", filePath, "\"."));
        }

        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        buffer = std::string(size, ' ');
        file.seekg(0);
        file.read(&buffer[0], size);

        return absl::OkStatus();
    }

    absl::Status Configuration::getHostDescriptions(HostDescriptionMap& hostDescriptions) const
    {
        hostDescriptions.clear();
        for(int i = 0; i < config.hosts_size(); i++)
        {
            const HostConfig& host = config.hosts(i);

            HostDescription hostDescription(host);

            if(hostDescription.isServer())
            {
                if(hostDescription.getHostServerAddress().empty())
                {
                    return absl::InvalidArgumentError(
                        absl::StrCat(
                            "Configuration: Host \"",
                            hostDescription.getHostname(),
                            "\" is defined as server, but has no server address specified."
                        )
                    );
                }
            }
            if(hostDescription.isClient())
            {
                if(hostDescription.getConnectToAddress().empty())
                {
                    return absl::InvalidArgumentError(
                        absl::StrCat(
                            "Configuration: Host \"",
                            hostDescription.getHostname(),
                            "\" is defined as client, but has no client address specified."
                        )
                    );
                }
            }

            if(host.hostname() == "")
            {
                return absl::InvalidArgumentError(
                        absl::StrCat(
                            "Configuration: Hostnmae not specified! At least one Host you added has no hostname. ",
                            "Please specify the hostname for each Host."
                        )
                    );
            }

            if(hostDescriptions.find(hostDescription.getHostname()) != hostDescriptions.end())
            {
                return absl::AlreadyExistsError(
                    absl::StrCat("Configuration: Host \"",
                    hostDescription.getHostname(), "\" was defined more than once."));
            }

            absl::Status status = hostDescriptions.insert(make_pair(hostDescription.getHostname(), hostDescription));
            if(!status.ok())
            {
                return status;
            }
        }
        return absl::OkStatus();
    }

    absl::Status Configuration::getModuleDescriptions(ModuleDescriptionMap& moduleDescriptions) const
    {
        moduleDescriptions.clear();
        Logger::logInfo("Hosts size %d \n", config.hosts_size());
        for(int i = 0; i < config.hosts_size(); i++)
        {
            const HostConfig& host = config.hosts(i);
            Logger::logInfo("Module size %d\n", host.modules_size());

            for(int j = 0; j < host.modules_size(); j++)
            {
                const ModuleConfig& moduleConfig = host.modules(j);

                ModuleDescription moduleDescription;
                moduleDescription.id = moduleConfig.id();
                moduleDescription.moduleClass = moduleConfig.type();
                moduleDescription.host = host.hostname();

                moduleDescription.properties = moduleConfig.properties();
      
                if(moduleConfig.id() == "")
                {
                    return absl::InvalidArgumentError(
                        absl::StrCat(
                            "Configuration: Module id not specified! At least one Module you added has no id. ",
                            "Please specify an id for each Module."
                        )
                    );
                }

                if(moduleConfig.type() == "")
                {
                    return absl::InvalidArgumentError(
                        absl::StrCat(
                            "Configuration: Module type not specified! The type was not specified for Module \"", moduleConfig.id(), "\". ",
                            "Please specify the type of each Module."
                        )
                    );
                }

                for(auto entry : moduleConfig.input_channels())
                {
                    moduleDescription.inputChannels.insert(make_pair(entry.first, entry.second));
                }

                for(auto entry : moduleConfig.output_channels())
                {
                    moduleDescription.outputChannels.insert(make_pair(entry.first, entry.second));
                }

                if(moduleDescriptions.find(moduleDescription.id) != moduleDescriptions.end())
                {
                    return absl::AlreadyExistsError(
                        absl::StrCat("Configuration: A Module with id \"",
                        moduleDescription.id, "\" was defined more than once."));
                }

                absl::Status status = moduleDescriptions.insert(make_pair(moduleDescription.id, moduleDescription));
                Logger::logInfo("Inserting module %s %s", moduleDescription.moduleClass .c_str(), moduleDescription.host.c_str());
                if(!status.ok())
                {
                    return status;
                }
            }
        }
        return absl::OkStatus();
    }

    absl::Status Configuration::extractModulesForHost(const string& hostId, const ModuleDescriptionMap& allModuleDescriptions, ModuleDescriptionMap& modulesForHost) const 
    {
        if(!hostExistsInConfiguration(hostId))
        {
            return absl::NotFoundError(absl::StrCat(
                "Could not setup host \"", hostId, "\". Host was not found in the configuration file"
            ));
        }       

        modulesForHost.clear();
        for(const auto& entry : allModuleDescriptions)
        {
            const ModuleDescription& moduleDescription = entry.second;
            
            if(moduleDescription.host == hostId)
            {
                modulesForHost[moduleDescription.id] = moduleDescription;
            }
        }
     
        return absl::OkStatus();
    }


    absl::Status Configuration::getChannelDescriptions(ChannelDescriptionMap& channelDescriptions) const
    {
        channelDescriptions.clear();
        for(int i = 0; i < config.hosts_size(); i++)
        {
            const HostConfig& host = config.hosts(i);

            for(int j = 0; j < host.modules_size(); j++)
            {
                const ModuleConfig& moduleConfig = host.modules(j);

                // We map channel names to connections.
                // For the middleware, it does not matter how channels are called, but what is the name of the "connection"
                // that they are mapped to. E.g., if a Channel is called "InputAudioData" and connected to "AudioDataStream",
                // then all the middleware needs to know that there is a subscriber for the "AudioDataStream".
                // During Module initialization, the middleware maps published/subscribed channels of
                // each Module automatically to the configured connection (check setChannelTypes function in module_table.cc).
                for(const auto& entry : moduleConfig.input_channels())
                {
                    channelDescriptions[entry.second].subscriberModules.push_back(moduleConfig.id());
                }

                for(const auto& entry : moduleConfig.output_channels())
                {
                    channelDescriptions[entry.second].publisherModules.push_back(moduleConfig.id());
                }
            }
        }

        for(auto& entry : channelDescriptions)
        {
            ChannelDescription& channelDescription = entry.second;
            // Storing the channel name (above we only defined the publisher and subsriberModules).
            channelDescription.channelName = entry.first;

            // Check if the channel has no publishers.
            if(channelDescription.publisherModules.empty())
            {
                // No error. It's fine to leave channels unconnected.
                Logger::logWarning("%s", 
                    absl::StrCat("Configuration: Channel \"",
                        channelDescription.channelName,
                        "\" has no publishers, therefore the subscribers would never receive data.").c_str());
                // return absl::InvalidArgumentError(
                //         absl::StrCat("Configuration: Channel \"",
                //         channelDescription.channelName,
                //         "\" has no publishers, therefore the subscribers would never receive data."));
            }

        }
        return absl::OkStatus();
    }

    bool Configuration::hostExistsInConfiguration(const std::string& host) const
    {
        for(auto& hostIt : config.hosts()) 
        {
            if (hostIt.hostname() == host) 
            {
                return true;
            }
        }
        return false;
    }

    void Configuration::getLogSinkConfiguration(LogSinkConfiguration& configuration, std::shared_ptr<SharedQueue<LogMessage>> logMessagesQueue) const
    {
        configuration.logSinkHost = this->config.log_sink_host();
        configuration.logSinkLogStoragePath = this->config.log_sink_log_storage_path();
        configuration.logSinkTransferMode = this->config.log_sink_transfer_mode();
        configuration.logSinkSeverityLevel = this->config.log_sink_severity_level();
        configuration.logSinkQueue = logMessagesQueue;
        // configuration.logSinkSeverityLevel = 
        //     this->config.has_log_sink_severity_level() ? 
        //         this->config.log_sink_severity_level() : LogMessageSeverityLevel::INFO;
    }


    LogMessageSeverityLevel Configuration::getMinLogSeverityLevelToPrint(const std::string& hostName) const
    {
        for(int i = 0; i < config.hosts_size(); i++)
        {
            const HostConfig& host = config.hosts(i);

            if(host.hostname() == hostName)
            {
                return host.min_log_severity_level();
            }
        }
        return LogMessageSeverityLevel::INFO;
    }

    bool Configuration::needToCheckIfAllModulesLoaded() const
    {
        return config.milliseconds_deadline_to_load_modules() >= 0;
    }

    int Configuration::getDeadlineForLoadingModulesInMs() const
    {
        if(config.milliseconds_deadline_to_load_modules() == 0)
        {
            // Default value.
            return 7500;
        }
        
        return config.milliseconds_deadline_to_load_modules();
    }

    // bool Configuration::isDesignerModeEnabled() const
    // {
    //     return config.designer_mode();
    // }
    
}
