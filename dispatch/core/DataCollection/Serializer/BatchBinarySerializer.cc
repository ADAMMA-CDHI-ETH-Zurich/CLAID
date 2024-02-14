#include "dispatch/core/DataCollection/Serializer/BatchBinarySerializer.hh"
#include "dispatch/core/DataCollection/Serializer/DataSerializerFactory.hh"


#include "google/protobuf/util/json_util.h"

namespace claid {

absl::Status BatchBinarySerializer::beginNewFile(const std::string& filePath) 
{
    Logger::logInfo("BatchBinarySerializer beginning file %s", filePath.c_str());
    this->currentFilePath = filePath;
    this->data = nullptr;
    return absl::OkStatus();
};

absl::Status BatchBinarySerializer::finishFile()
{
    if(this->data == nullptr)
    {
        return absl::InvalidArgumentError("BatchBinarySerializer failed to finish file. Current data is null.");
    }

    std::ofstream outputFile(this->currentFilePath, std::ios::app);

    if(!outputFile.is_open())
    {
        return absl::UnavailableError(absl::StrCat("BatchBinarySerializer failed to open file \"", this->currentFilePath, "\" for writing"));
    }

    std::string serializedData = this->data->SerializeAsString();

    outputFile.write(serializedData.c_str(), serializedData.size());
    return absl::OkStatus();
}

absl::Status BatchBinarySerializer::onNewData(std::shared_ptr<const google::protobuf::Message> newData)
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

REGISTER_DATA_SERIALIZER(BatchBinarySerializer, claid::BatchBinarySerializer, std::vector<std::string>({"BATCH_BINARY", "batch_binary"}));