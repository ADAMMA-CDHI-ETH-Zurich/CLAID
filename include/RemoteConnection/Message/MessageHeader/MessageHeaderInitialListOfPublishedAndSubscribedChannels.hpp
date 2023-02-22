#pragma once
#include "RemoteConnection/Message/MessageHeader/MessageHeader.hpp"

namespace claid
{
    namespace RemoteConnection
    {
        struct MessageHeaderInitialListOfPublishedAndSubscribedChannels : public MessageHeader
        {

            template<typename Reflector>
            void reflect(Reflector& r)
            {
                
            }

            
            
        };
    }
}