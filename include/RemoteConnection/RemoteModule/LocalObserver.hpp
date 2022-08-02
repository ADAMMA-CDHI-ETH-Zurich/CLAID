#pragma once

#include "RunTime/RunTime.hpp"

namespace portaible
{
    // Observes everything we do locally (i.e. channels 
    // that were (un)published or (un)subscribed within the RunTime (global ChannelManager)).
    class LocalObserver : public SubModule
    {
        private:
            // Does not receive data from network, but is able to send data over network.
            Channel<std::string> subscribedChannel;
            Channel<std::string> publishedChannel;
            Channel<std::string> unsubscribedChannel;
            Channel<std::string> unpublishedChannel;

            void onChannelSubscribed(ChannelData<std::string> channelID);
            void onChannelPublished(ChannelData<std::string> channelID);
            void onChannelUnsubscribed(ChannelData<std::string> channelID);
            void onChannelUnpublished(ChannelData<std::string> channelID);

        public:
           void observe(ChannelManager* manager);

    };
}