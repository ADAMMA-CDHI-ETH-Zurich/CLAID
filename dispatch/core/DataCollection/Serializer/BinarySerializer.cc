#include "dispatch/core/DataCollection/Serializer/BinarySerializer.hh"
#include "dispatch/core/DataCollection/Serializer/DataSerializerFactory.hh"


#include "google/protobuf/util/json_util.h"

namespace claid {

absl::Status BinarySerializer::beginNewFile(const std::string& filePath) 
{
    Logger::printfln("BinarySerializer beginning file %s", filePath.c_str());
    this->currentFilePath = filePath;
    this->data = nullptr;
    return absl::OkStatus();
};

absl::Status BinarySerializer::finishFile()
{
    if(this->data == nullptr)
    {
        return absl::InvalidArgumentError("BinarySerializer failed to finish file. Current data is null.");
    }

    std::ofstream outputFile(this->currentFilePath, std::ios::app);

    if(!outputFile.is_open())
    {
        return absl::UnavailableError(absl::StrCat("BinarySerializer failed to open file \"", this->currentFilePath, "\" for writing"));
    }

    std::string serializedData = this->data->SerializeAsString();

    outputFile.write(serializedData.c_str(), serializedData.size());
    return absl::OkStatus();
}

absl::Status BinarySerializer::onNewData(std::shared_ptr<const google::protobuf::Message> newData)
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

REGISTER_DATA_SERIALIZER(BinarySerializer, claid::BinarySerializer, std::vector<std::string>({"BINARY", "binary"}));