#pragma once

#include "RemoteConnection/Message/MessageData/MessageData.hpp"

namespace claid
{
    namespace RemoteConnection
    {
        struct MessageDataEmpty : public MessageData
        {
            //DECLARE_SERIALIZATION(MessageDataEmpty)

            template<typename Reflector>
            void reflect(Reflector& r)
            {
            }

            
        };
    }
}