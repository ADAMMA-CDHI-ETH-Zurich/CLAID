#pragma once
#include "RemoteConnection/Message/MessageHeader/MessageHeader.hpp"

namespace claid
{
    namespace RemoteConnection
    {
        struct MessageHeaderChannelUpdate : public MessageHeader
        {
            //DECLARE_SERIALIZATION(MessageHeaderChannelUpdate)

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