#pragma once
#include "Serialization/Serialization.hpp"
#include "Binary/BinaryData.hpp"

namespace portaible
{
    namespace Network
    {
        struct MessageDataBinary
        {
            DECLARE_SERIALIZATION(MessageDataBinary)
            BinaryData binaryData;

            template<typename Reflector>
            void reflect(Reflector& r)
            {
                r.member("BinaryData", binaryData, "");
            }
            
        };
    }
}