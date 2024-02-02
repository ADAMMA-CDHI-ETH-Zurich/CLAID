#pragma once

#include "google/protobuf/message.h"
#include "dispatch/proto/claidservice.pb.h"

namespace claid {

class AnyProtoType
{
    private:
        std::shared_ptr<google::protobuf::Message> message;

    public:
        AnyProtoType()
        {
            // If a Module subscribes/publihses a Channel using AnyProtoType, 
            // then during the initialization of the Module, an example DataPackage will be created,
            // which is used to tell the Middleware what data is expected by the Module.
            // Hence, during publish/subscribe, a Mutator will be created for AnyProtoType, which will be used
            // to set an example instance for a DataPackage.
            // For this to work, message cannot be null.
            // Hence, in the case of the default constructor, we have to initialize message with a valid proto type.
            // The type itself does not matter, because all proto types are represented by DataPackage::PayloadOneOfCase::kBlobVal.
            // Therefore, we choose NumberArray here, because it is a small data type with only minimal overhead.
            message = std::static_pointer_cast<google::protobuf::Message>(std::make_shared<google::protobuf::Empty>());
        }

        AnyProtoType(std::shared_ptr<google::protobuf::Message> message) : message(message)
        {

        }

        template<typename T>
        AnyProtoType(std::shared_ptr<T> message)
        {
            this->message = std::static_pointer_cast<google::protobuf::Message>(message);
        }

        void setMessage(std::shared_ptr<google::protobuf::Message> message)
        {
            this->message = message;
        }

        std::shared_ptr<const google::protobuf::Message> getMessage() const
        {
            return this->message;
        }
};

}