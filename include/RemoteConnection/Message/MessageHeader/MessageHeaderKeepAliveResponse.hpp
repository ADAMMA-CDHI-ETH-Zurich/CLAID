#pragma once
#include "RemoteConnection/Message/MessageHeader/MessageHeader.hpp"

namespace claid
{
    namespace RemoteConnection
    {
        // Answer message to MessageHeaderKeepAlive. See there for more details.
        struct MessageHeaderKeepAliveResponse : public MessageHeader
        {
            DECLARE_SERIALIZATION(MessageHeaderKeepAliveResponse)

            template<typename Reflector>
            void reflect(Reflector& r)
            {
                
            }
        };
    }
}