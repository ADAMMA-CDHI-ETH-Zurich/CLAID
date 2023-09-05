#include "dispatch/core/CLAIDConfig/CLAIDConfig.hh"


#include "google/protobuf/util/json_util.h"


#include <fstream>

namespace claid
{
    bool loadConfigFileToString(const std::string& filePath, std::string& buffer)
    {
        std::ifstream file(filePath);

        if(!file.is_open())
        {
            return false;
        }

        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        buffer = std::string(size, ' ');
        file.seekg(0);
        file.read(&buffer[0], size); 

        return true;
    }

    absl::Status parseConfigFile(const std::string& configFilePath, shared_ptr<claidservice::CLAIDConfig>& configPtr)
    {
        std::string configContent;
        if(!loadConfigFileToString(configFilePath, configContent))
        {
            return absl::InvalidArgumentError("Could not open configuration file " + configFilePath);
        }



        configPtr = make_shared<claidservice::CLAIDConfig>();

        google::protobuf::util::JsonParseOptions options2;
        absl::Status status = JsonStringToMessage(configContent, configPtr.get(), options2);

        return status;
    }

    absl::Status configToJson(std::shared_ptr<claidservice::CLAIDConfig> config, std::string& buffer)
    {
        buffer = "";
        if(config.get() == nullptr)
        {
            return absl::InvalidArgumentError("Cannot serialize CLAIDConfig to json. Provided configuration is null.");
        }

        google::protobuf::util::JsonPrintOptions options;
        options.add_whitespace = true;
        options.always_print_primitive_fields = true;
        options.preserve_proto_field_names = true;
        return MessageToJsonString(*config.get(), &buffer, options);
    }

}
