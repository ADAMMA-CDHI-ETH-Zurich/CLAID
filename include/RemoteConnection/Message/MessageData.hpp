#pragma once
#include "Serialization/Serialization.hpp"

namespace portaible
{
    namespace RemoteConnection
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