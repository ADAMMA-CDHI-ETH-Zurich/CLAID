#pragma once

#include "RunTime/RunTime.hpp"
#include "RemoteConnection/Message/Message.hpp"


namespace portaible
{
    // Observes everything we do locally (i.e. channels 
    // that were (un)published or (un)subscribed within the RunTime (global ChannelManager)).
    class LocalObserver : public SubModule
    {
        private:
            ChannelManager* observedChannelManager = nullptr;
            Channel<RemoteConnection::Message> sendMessageChannel;

            // Does not receive data from network, but is able to send data over network.
            Channel<std::string> subscribedChannel;
            Channel<std::string> publishedChannel;
            Channel<std::string> unsubscribedChannel;
            Channel<std::string> unpublishedChannel;

            void onChannelSubscribedCallback(ChannelData<std::string> channelID);
            void onChannelPublishedCallback(ChannelData<std::string> channelID);
            void onChannelUnsubscribedCallback(ChannelData<std::string> channelID);
            void onChannelUnpublishedCallback(ChannelData<std::string> channelID);

            void onChannelSubscribed(const std::string& channelID);
            void onChannelPublished(const std::string& channelID);
            void onChannelUnsubscribed(const std::string& channelID);
            void onChannelUnpublished(const std::string& channelID);
        public:
            LocalObserver(Channel<RemoteConnection::Message> sendMessageChannel);


           void observe(ChannelManager* manager);

    };
}