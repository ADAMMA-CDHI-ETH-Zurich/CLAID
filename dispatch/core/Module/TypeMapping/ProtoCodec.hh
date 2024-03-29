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

#include <iostream>
#include <functional>
#include <string>
#include <vector>
#include <stdexcept>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/any.pb.h>


#include "dispatch/core/Logger/Logger.hh"
#include "dispatch/proto/claidservice.pb.h"

using claidservice::Blob;
using claidservice::Codec;


namespace claid {

class ProtoCodec {
private:
    std::string fullName;
    std::shared_ptr<const google::protobuf::Message> msg;



public:
    
    ProtoCodec()
    {

    }
    ProtoCodec(std::shared_ptr<const google::protobuf::Message> message) 
        : msg(message)  
    {
        if(msg != nullptr)
        {
            this->fullName = msg->GetDescriptor()->full_name();
        }
        else
        {
            throw std::invalid_argument("Msg is null in ProtoCodec");
        }
    }



    bool encode(const google::protobuf::Message* protoMessage, Blob& returnBlob) 
    {
        const std::string name = protoMessage->GetDescriptor()->full_name();
        if(name != fullName)
        {
            return false;
        }

        std::string serializedData = protoMessage->SerializeAsString();

        returnBlob.set_codec(Codec::CODEC_PROTO);
        returnBlob.set_payload(serializedData);

        returnBlob.set_message_type(fullName);

        return true;
    }

    bool decode(const Blob& blob, google::protobuf::Message* returnValue) 
    {
        if(returnValue == nullptr)
        {
            Logger::logError("Failed to parse protobuf message from Blob. The returnValue is null.");
            return false;
        }
     
        const std::string name = returnValue->GetDescriptor()->full_name();
        if(name != fullName)
        {
            return false;
        }
        try 
        {

            if (!returnValue->ParseFromString(blob.payload())) 
            {
                Logger::logError("Failed to parse protobuf message of type. ParseFromString failed.");
                return false;
            }

            return true;
        } 
        catch (const std::exception& e) 
        {
            // Replace with actual error handling/logging
            Logger::logError("Failed to parse protobuf message from Blob: %s", e.what());
            return false;
        }
    }

    std::shared_ptr<google::protobuf::Message> decodeIntoNewInstance(const Blob& blob)
    {
        if(this->msg == nullptr)
        {
            return nullptr;
        }

        std::shared_ptr<google::protobuf::Message> newInstance(this->msg->New());

        if(!decode(blob, newInstance.get()))
        {
            return nullptr;
        }

        return newInstance;
    }
};
}
