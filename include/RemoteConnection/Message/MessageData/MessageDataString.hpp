#pragma once

#include "RemoteConnection/Message/MessageData/MessageData.hpp"

namespace claid
{
    namespace RemoteConnection
    {
        struct MessageDataString : public MessageData
        {
            //DECLARE_SERIALIZATION(MessageDataString)

            template<typename Reflector>
            void reflect(Reflector& r)
            {
                r.member("Data", this->string, "");
            }

            std::string string;
            
        };
    }
}