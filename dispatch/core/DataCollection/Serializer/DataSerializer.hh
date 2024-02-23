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