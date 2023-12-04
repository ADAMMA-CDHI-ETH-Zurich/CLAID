#include "dispatch/core/DataCollection/Serializer/JSONSerializer.hh"
#include "dispatch/core/DataCollection/Serializer/DataSerializerFactory.hh"


#include "google/protobuf/util/json_util.h"

namespace claid {

absl::Status JSONSerializer::beginNewFile(const std::string& filePath) 
{
    this->currentFilePath = filePath;
    this->data = nullptr;
    return absl::OkStatus();
};

absl::Status JSONSerializer::finishFile()
{
    if(this->data == nullptr)
    {
        return absl::InvalidArgumentError("JSONSerializer failed to finish file. Current data is null.");
    }

    std::ofstream outputFile(this->currentFilePath, std::ios::app);

    if(!outputFile.is_open())
    {
        return absl::UnavailableError(absl::StrCat("JSONSerializer failed to open file \"", this->currentFilePath, "\" for writing"));
    }

    std::string jsonOutput = "";
    google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = true;
    options.always_print_primitive_fields = true;
    options.preserve_proto_field_names = true;
    absl::Status status = MessageToJsonString(*this->data, &jsonOutput, options);

    return absl::OkStatus();
}

absl::Status JSONSerializer::onNewData(std::shared_ptr<const google::protobuf::Message> newData)
{
    if(this->data == nullptr)
    {
        google::protobuf::Message* msg = newData->New();
        this->data = std::shared_ptr<google::protobuf::Message>(msg);
    
        // Merge the fields from the original message into the new message
        this->data->MergeFrom(*newData);
    }
    else
    {
        absl::Status status = this->mergeMessages(*this->data, *newData);

        if(!status.ok())
        {
            return status;
        }
    }

    return absl::OkStatus();
}

}

REGISTER_DATA_SERIALIZER(JSONSerializer, claid::JSONSerializer, std::vector<std::string>({"JSON", "json"}));