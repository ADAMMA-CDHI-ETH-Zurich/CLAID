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

            template<typename T>
            T* as()
            {
                return static_cast<T*>(this);
            }
        };
    }
}