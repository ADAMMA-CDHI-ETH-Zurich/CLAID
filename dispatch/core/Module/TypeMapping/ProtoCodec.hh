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



    void encode(const google::protobuf::Message* protoMessage, Blob& returnBlob) 
    {
        std::string serializedData = protoMessage->SerializeAsString();

        returnBlob.set_codec(Codec::CODEC_PROTO);
        returnBlob.set_payload(serializedData);

        returnBlob.set_message_type(fullName);
    }

    bool decode(const Blob& blob, google::protobuf::Message* returnValue) 
    {
        if(returnValue == nullptr)
        {
            Logger::logError("Failed to parse protobuf message from Blob. The returnValue is nullptr");
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
};
}
