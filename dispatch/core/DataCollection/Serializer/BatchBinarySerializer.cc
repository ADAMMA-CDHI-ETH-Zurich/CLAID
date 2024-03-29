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

    std::ofstream outputFile(this->currentFilePath, (this->overrideExistingFiles ? (std::ios::out) : (std::ios::app)) | std::ios::binary);

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