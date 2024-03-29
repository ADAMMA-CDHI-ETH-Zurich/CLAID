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

#pragma once

#include <string>
#include <google/protobuf/message.h>

#include "dispatch/core/Logger/Logger.hh"
#include "absl/status/status.h"
#include "absl/strings/str_cat.h"


namespace claid {

class DataSerializer
{
    protected:
        // Function to check if a field is repeated
        bool containsOnlyRepeatedFields(const google::protobuf::Message& message, std::string& nonRepatedFieldName) 
        {
            const google::protobuf::Descriptor* descriptor = message.GetDescriptor();

            // Check if all fields are repeated
            for (int i = 0; i < descriptor->field_count(); ++i) 
            {
                const google::protobuf::FieldDescriptor* field = descriptor->field(i);

                if (!field->is_repeated()) 
                {
                    const std::string messageType = message.GetDescriptor()->full_name();

                    nonRepatedFieldName = field->name();

                    return false;
                }
            }
            return true;
        }

        // Function to merge two Protocol Buffers messages with only repeated fields
        absl::Status mergeMessages(google::protobuf::Message& message1,
                                                const google::protobuf::Message& message2) {
            // Check if the messages are of the same type
            if (message1.GetDescriptor() != message2.GetDescriptor()) 
            {
                const std::string type1 = message1.GetDescriptor()->full_name();
                const std::string type2 = message2.GetDescriptor()->full_name();
                
                return absl::InvalidArgumentError(absl::StrCat("Failed to merge messages, messages are not of the same type.\n"
                                "Type of first message is \"", type1, "\", type of second message is \"", type2));
            }

            std::string nonRepeatedFieldName;
            if(!containsOnlyRepeatedFields(message1, nonRepeatedFieldName))
            {
                return absl::InvalidArgumentError(absl::StrCat("Failed to merge messages. Message of type \"", message1.GetDescriptor()->full_name(), 
                    "\" contains non-repeated field \"", nonRepeatedFieldName, "\".\n",
                    "Merge is allowed only for messages with only repeated fields.\n"));
            }
            
            if(!containsOnlyRepeatedFields(message2, nonRepeatedFieldName))
            {
                return absl::InvalidArgumentError(absl::StrCat("Failed to merge messages. Message of type \"", message1.GetDescriptor()->full_name(), 
                    "\" contains non-repeated field \"", nonRepeatedFieldName, "\".\n",
                    "Merge is allowed only for messages with only repeated fields.\n"));
            }
            

            // Create a new message of the same type
            std::shared_ptr<google::protobuf::Message> mergedMessage(message1.New());

            // Merge the fields from both messages into the new message
            message1.MergeFrom(message2);

            return absl::OkStatus();
        }

        bool overrideExistingFiles = false;

    public:

        virtual ~DataSerializer() {}
        
        void setOverrideExistingFiles(bool overrideExistingFiles)
        {
            this->overrideExistingFiles = overrideExistingFiles;
        }

        virtual absl::Status beginNewFile(const std::string& file) = 0;
        virtual absl::Status finishFile() = 0;

        virtual absl::Status onNewData(std::shared_ptr<const google::protobuf::Message> data) = 0;
};

}