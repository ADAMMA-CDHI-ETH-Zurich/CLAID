#pragma once
#include "Serialization/Serialization.hpp"

namespace portaible
{
    namespace Network
    {
        struct MessageHeader
        {
            DECLARE_SERIALIZATION(MessageHeader)

            template<typename Reflector>
            void reflect(Reflector& r)
            {
                
            }
            
        };
    }
}