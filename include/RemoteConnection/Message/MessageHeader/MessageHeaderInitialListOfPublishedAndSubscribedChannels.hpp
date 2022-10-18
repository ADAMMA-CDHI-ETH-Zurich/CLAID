#pragma once
#include "RemoteConnection/Message/MessageHeader/MessageHeader.hpp"

namespace claid
{
    namespace RemoteConnection
    {
        struct MessageHeaderInitialListOfPublishedAndSubscribedChannels : public MessageHeader
        {
            DECLARE_SERIALIZATION(MessageHeaderInitialListOfPublishedAndSubscribedChannels)

            template<typename Reflector>
            void reflect(Reflector& r)
            {
                
            }

            
            
        };
    }
}