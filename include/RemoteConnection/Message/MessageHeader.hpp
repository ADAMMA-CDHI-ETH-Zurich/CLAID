#pragma once
#include "Serialization/Serialization.hpp"

namespace portaible
{
    namespace RemoteConnection
    {
        struct MessageHeader
        {
            DECLARE_SERIALIZATION(MessageHeader)

            template<typename Reflector>
            void reflect(Reflector& r)
            {
                r.member("Description", description, "");
            }

            std::string description;
            
        };
    }
}