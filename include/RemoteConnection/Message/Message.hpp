#pragma once

#include <memory.h>
#include "RemoteConnection/Message/MessageHeader/MessageHeader.hpp"
#include "RemoteConnection/Message/MessageData/MessageData.hpp"
#include "Serialization/Serialization.hpp"
#include "Binary/BinaryData.hpp"

namespace portaible
{
    namespace RemoteConnection
    {
        struct Message
        {
            std::shared_ptr<MessageHeader> header;
            std::shared_ptr<MessageData> data;

            template<typename HeaderType, typename DataType>
            static Message CreateMessage()
            {
                Message message;
                message.header = 
                    std::static_pointer_cast<MessageHeader>(std::shared_ptr<HeaderType>(new HeaderType));

                message.data = 
                    std::static_pointer_cast<MessageData>(std::shared_ptr<DataType>(new DataType));
                
                return message;
            }


            template<typename Reflector>
            void reflect(Reflector& r)
            {
                r.member("Header", header, "");
                r.member("Data", data, "");
            }
        };
    }
}