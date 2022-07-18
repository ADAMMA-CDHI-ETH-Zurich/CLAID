#pragma once

#include <memory.h>
#include "MessageHeader.hpp"
#include "MessageData.hpp"
#include "Serialization/Serialization.hpp"
#include "Binary/BinaryData.hpp"

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

            template<typename Reflector>
            void reflect(Reflector& r)
            {
                r.member("Header", header, "");
                r.member("Data", data, "");
            }
        };
    }
}