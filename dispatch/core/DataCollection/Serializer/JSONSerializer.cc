#include "dispatch/core/DataCollection/Serializer/JSONSerializer.hh"
#include "dispatch/core/DataCollection/Serializer/DataSerializerFactory.hh"


#include "google/protobuf/util/json_util.h"

namespace claid {

absl::Status JSONSerializer::beginNewFile(const std::string& filePath) 
{
    Logger::logInfo("JSONSerializer beginning file %s", filePath.c_str());
    this->currentFilePath = filePath;
    this->outputFile = std::ofstream(this->currentFilePath, std::ios::app);
    if(!outputFile.is_open())
    {
        return absl::UnavailableError(absl::StrCat("JSONSerializer failed to open file \"", this->currentFilePath, "\" for writing"));
    }

    // this->data = nullptr;
    return absl::OkStatus();
};

absl::Status JSONSerializer::finishFile()
{
    // if(this->data == nullptr)
    // {
    //     return absl::InvalidArgumentError("JSONSerializer failed to finish file. Current data is null.");
    // }


    this->outputFile.close();
    return absl::OkStatus();
}

absl::Status JSONSerializer::onNewData(std::shared_ptr<const google::protobuf::Message> newData)
{
    // if(this->data == nullptr)
    // {
    //     google::protobuf::Message* msg = newData->New();
    //     this->data = std::shared_ptr<google::protobuf::Message>(msg);
    
    //     // Merge the fields from the original message into the new message
    //     this->data->MergeFrom(*newData);
    // }
    // else
    // {
    //     absl::Status status = this->mergeMessages(*this->data, *newData);

    //     if(!status.ok())
    //     {
    //         return status;
    //     }
    // }

    std::string jsonOutput = "";
    google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = true;
    options.always_print_primitive_fields = true;
    options.preserve_proto_field_names = true;
    absl::Status status = MessageToJsonString(*newData, &jsonOutput, options);

    jsonOutput += "\n";
    outputFile << jsonOutput;
    this->outputFile.flush();

    return absl::OkStatus();
}

}

REGISTER_DATA_SERIALIZER(JSONSerializer, claid::JSONSerializer, std::vector<std::string>({"JSON", "json"}));