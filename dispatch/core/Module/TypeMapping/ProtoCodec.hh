#include <iostream>
#include <functional>
#include <string>
#include <vector>
#include <stdexcept>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/any.pb.h>

#include "dispatch/proto/claidservice.pb.h"



class ProtoCodec {
private:
    const std::string fullName;
    const google::protobuf::Message* msg;

    typedef std::function<(const google::protobuf::Message*, Blob&)> EncoderFunction;
    typedef std::function<google::protobuf::Message*(const Blob&)> DecoderFunction;

    EncoderFunction encoder;
    DecoderFunction decoder;

public:
    ProtoCodec(const google::protobuf::Message* msg) 
        : msg(msg), encoder(buildEncoder()), decoder(buildDecoder())    
    {
        if(msg != nullptr)
        {
            this->fullName = msg->GetDescriptor()->full_name();
        }
    }

    EncoderFunction buildEncoder() {
        return [this](const google::protobuf::Message* protoMessage, Blob& returnBlob) ->  {
            std::string serializedData = protoMessage->SerializeAsString();

            returnBlob.set_codec(Codec::CODEC_PROTO);
            returnBlob.set_payload(serializedData);
            returnBlob.set_message_type(fullName);

        };
    }

    DecoderFunction buildDecoder() {
        google::protobuf::Message* message = msg->New();

        return [this, message](const Blob& blob) -> google::protobuf::Message* {
            try {
                if (!message->ParseFromString(blob.payload())) {
                    throw std::invalid_argument("Failed to parse message from blob");
                }
                return message;
            } catch (const std::exception& e) {
                // Replace with actual error handling/logging
                std::cerr << "Error: " << e.what() << std::endl;
                return nullptr;
            }
        };
    }

    void encode(const google::protobuf::Message* t, Blob& returnBlob) 
    {
        encoder(t, returnBlob);
    }

    google::protobuf::Message* decode(const Blob& blob) {
        return decoder(blob);
    }
};

// Define Blob, Codec, and Logger classes as needed
