#include "dispatch/core/Configuration/Configuration.hh"


#include "google/protobuf/util/json_util.h"


#include <fstream>

using claidservice::HostConfig;
using claidservice::ModuleConfig;

namespace claid
{

    absl::Status Configuration::fromJSONString(const std::string& json)
    {
        google::protobuf::util::JsonParseOptions options2;
        absl::Status status = JsonStringToMessage(json, &this->config, options2);
        
        return status;
    }

    absl::Status Configuration::toJSONString(std::string& jsonOutput) const
    {
        jsonOutput = "";
        google::protobuf::util::JsonPrintOptions options;
        options.add_whitespace = true;
        options.always_print_primitive_fields = true;
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

            HostDescription hostDescription;
            hostDescription.hostname = host.hostname();
            hostDescription.isServer = host.is_server();
            hostDescription.hostServerAddress = host.host_server_address();
            hostDescription.connectTo = host.connect_to();

            if(hostDescriptions.find(hostDescription.hostname) != hostDescriptions.end())
            {
                return absl::AlreadyExistsError(
                    absl::StrCat("Configuration: Host \"", 
                    hostDescription.hostname, "\" was defined more than once."));
            }

            absl::Status status = hostDescriptions.insert(make_pair(hostDescription.hostname, hostDescription));
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
        for(int i = 0; i < config.hosts_size(); i++)
        {
            const HostConfig& host = config.hosts(i);

            for(int j = 0; j < host.modules_size(); j++)
            {
                const ModuleConfig& moduleConfig = host.modules(j);

                ModuleDescription moduleDescription;
                moduleDescription.id = moduleConfig.id();
                moduleDescription.moduleClass = moduleConfig.type();

                for(auto entry : moduleConfig.properties())
                {
                    moduleDescription.properties.insert(make_pair(entry.first, entry.second));
                }

                moduleDescription.inputChannels = 
                    std::vector<std::string>(moduleDescription.inputChannels.begin(), moduleDescription.inputChannels.end());
                
                moduleDescription.outputChannels = 
                    std::vector<std::string>(moduleDescription.outputChannels.begin(), moduleDescription.outputChannels.end());

                if(moduleDescriptions.find(moduleDescription.id) != moduleDescriptions.end())
                {
                    return absl::AlreadyExistsError(
                        absl::StrCat("Configuration: A Module with id \"", 
                        moduleDescription.id, "\" was defined more than once."));
                }

                absl::Status status = moduleDescriptions.insert(make_pair(moduleDescription.id, moduleDescription));
                if(!status.ok())
                {
                    return status;
                }
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

                for(const std::string& inputChannel : moduleConfig.input_channels())
                {
                    channelDescriptions[inputChannel].subscriberModules.push_back(moduleConfig.id());
                }

                for(const std::string& outputChannel : moduleConfig.output_channels())
                {
                    channelDescriptions[outputChannel].publisherModules.push_back(moduleConfig.id());
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
                return absl::InvalidArgumentError(
                        absl::StrCat("Configuration: Channel \"", 
                        channelDescription.channelName, 
                        "\" has no publishers, therefore the subscribers would never receive data."));
            }
            
        }
        return absl::OkStatus();
    }
}
