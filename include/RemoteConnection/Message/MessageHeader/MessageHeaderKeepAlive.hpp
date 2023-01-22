#pragma once
#include "RemoteConnection/Message/MessageHeader/MessageHeader.hpp"

namespace claid
{
    namespace RemoteConnection
    {
        // Message that is sent regularly to check whether the TCP connection is still alive.
        // Needs to be replied to with MessageHeaderKeepAliveResponse in a certain time interval 
        // (typically set by RemoteObserver). Otherwise, connection is considered dead.
        struct MessageHeaderKeepAlive : public MessageHeader
        {
            DECLARE_SERIALIZATION(MessageHeaderKeepAlive)

            template<typename Reflector>
            void reflect(Reflector& r)
            {
                
            }
        };
    }
}