#include "dispatch/core/DataCollection/Serializer/BatchJSONSerializer.hh"
#include "dispatch/core/DataCollection/Serializer/DataSerializerFactory.hh"


#include "google/protobuf/util/json_util.h"

namespace claid {

absl::Status BatchJSONSerializer::beginNewFile(const std::string& filePath) 
{
    Logger::logInfo("BatchJSONSerializer beginning file %s", filePath.c_str());
    this->currentFilePath = filePath;
    this->data = nullptr;
    return absl::OkStatus();
};

absl::Status BatchJSONSerializer::finishFile()
{
    if(this->data == nullptr)
    {
        return absl::InvalidArgumentError("BatchJSONSerializer failed to finish file. Current data is null.");
    }

    std::ofstream outputFile(this->currentFilePath, std::ios::app);

    if(!outputFile.is_open())
    {
        return absl::UnavailableError(absl::StrCat("BatchJSONSerializer failed to open file \"", this->currentFilePath, "\" for writing"));
    }

    std::string jsonOutput = "";
    google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = true;
    options.always_print_primitive_fields = true;
    options.preserve_proto_field_names = true;
    absl::Status status = MessageToJsonString(*this->data, &jsonOutput, options);

    outputFile << jsonOutput;
    return absl::OkStatus();
}

absl::Status BatchJSONSerializer::onNewData(std::shared_ptr<const google::protobuf::Message> newData)
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

REGISTER_DATA_SERIALIZER(BatchBatchJSONSerializer, claid::BatchJSONSerializer, std::vector<std::string>({"BATCH_JSON", "batch_json"}));