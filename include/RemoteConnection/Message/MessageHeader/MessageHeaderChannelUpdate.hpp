#pragma once
#include "RemoteConnection/Message/MessageHeader/MessageHeader.hpp"

namespace portaible
{
    namespace RemoteConnection
    {
        struct MessageHeaderChannelUpdate
        {
            DECLARE_SERIALIZATION(MessageHeaderChannelUpdate)

            enum UpdateType
            {
                CHANNEL_PUBLISHED,
                CHANNEL_SUBSCRIBED,
                CHANNEL_UNPUBLISHED,
                CHANNEL_UNSUBSCRIBED,
            };

            template<typename Reflector>
            void reflect(Reflector& r)
            {
                r.member("UpdateType", updateType, "");
            }

            UpdateType updateType;
            
        };
    }
}