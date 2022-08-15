#pragma once

#include "RemoteConnection/Message/MessageData/MessageData.hpp"

namespace portaible
{
    namespace RemoteConnection
    {
        struct MessageData2DStringVector : public MessageData
        {
            DECLARE_SERIALIZATION(MessageData2DStringVector)

            template<typename Reflector>
            void reflect(Reflector& r)
            {
                r.member("StringLists", this->stringLists, "");
            }

            std::vector<std::vector<std::string>> stringLists;
            
        };
    }
}