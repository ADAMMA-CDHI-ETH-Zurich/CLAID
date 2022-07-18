#pragma once
#include "Serialization/Serialization.hpp"

namespace portaible
{
    namespace Network
    {
        struct MessageData
        {
            DECLARE_SERIALIZATION(MessageData)

            template<typename Reflector>
            void reflect(Reflector& r)
            {
                
            }
        };
    }
}