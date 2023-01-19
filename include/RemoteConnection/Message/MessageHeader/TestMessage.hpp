#pragma once
#include "RemoteConnection/Message/MessageHeader/MessageHeader.hpp"

namespace claid
{
    namespace RemoteConnection
    {
        struct TestMessage : public MessageHeader
        {
            DECLARE_SERIALIZATION(TestMessage)

            template<typename Reflector>
            void reflect(Reflector& r)
            {
                
            }

            
            
        };
    }
}