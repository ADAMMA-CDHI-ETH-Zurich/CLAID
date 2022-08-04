#pragma once

#include "RunTime/RunTime.hpp"
#include "RemoteConnection/Message/Message.hpp"
#include "RemoteConnection/Message/MessageHeader/MessageHeaderChannelUpdate.hpp"
#include "RemoteConnection/Message/MessageData/MessageDataString.hpp"

namespace portaible
{
    namespace RemoteConnection
    {
        // Observes everything we do locally (i.e. channels 
        // that were (un)published or (un)subscribed within the RunTime (global ChannelManager)).
        class RemoteObserver : public SubModule
        {
            private:
                void onMessageReceived(ChannelData<Message> message);
                void onChannelUpdate(MessageHeaderChannelUpdate::UpdateType updateType, const MessageDataString& data);
                void onChannelPublished(const std::string& channelID);

            public:
                ChannelSubscriber<Message> getSubscriberForReceptionOfMessages();

     

                template<typename Header, typename Data, typename Class, typename... arguments>
                void callIf(const MessageHeader& header, const MessageData& dat, void (Class::*f)(arguments...), Class* obj, arguments... args)
                {
                    if(header.is<Header>())
                    {
                        Header& typedHeader = *header.as<Header>();


                    }
                }

        };
    }
}