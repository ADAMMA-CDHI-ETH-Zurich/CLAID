#pragma once
#include "RemoteConnection/Message/MessageHeader/MessageHeader.hpp"
#include "Channel/ChannelData.hpp"
namespace portaible
{
    namespace RemoteConnection
    {
        struct MessageHeaderChannelData : public MessageHeader
        {
            DECLARE_SERIALIZATION(MessageHeaderChannelData)


            template<typename Reflector>
            void reflect(Reflector& r)
            {
                r.member("TargetChannel", targetChannel, "");
                r.member("Header", header, "");
            }

            // Channel to post data to.
            std::string targetChannel;
            TaggedDataBase header;
            
            
        };
    }
}