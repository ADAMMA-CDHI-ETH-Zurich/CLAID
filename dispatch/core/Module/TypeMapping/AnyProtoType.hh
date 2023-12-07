#pragma once

#include "google/protobuf/message.h"

namespace claid {

class AnyProtoType
{
    private:
        std::shared_ptr<google::protobuf::Message> message;

    public:
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