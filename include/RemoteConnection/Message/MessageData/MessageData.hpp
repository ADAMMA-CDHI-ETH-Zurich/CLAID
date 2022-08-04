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
            bool is() const
            {
                return dynamic_cast<const T*>(this) != nullptr;
            }

            template<typename T>
            T* as()
            {
                return static_cast<T*>(this);
            }

            template<typename T>
            const T* as() const
            {
                return static_cast<const T*>(this);
            }
        };
    }
}