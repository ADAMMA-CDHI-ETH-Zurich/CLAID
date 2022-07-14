#pragma once

#include <memory.h>
#include "MessageHeader.hpp"
#include "MessageData.hpp"



namespace portaible
{
    namespace Network
    {
        struct Message
        {
            std::shared_ptr<MessageHeader> header;
            std::shared_ptr<MessageData> data;

            void toBinary(BinaryData& binaryData)
            {
                
            }
        };
    }
}