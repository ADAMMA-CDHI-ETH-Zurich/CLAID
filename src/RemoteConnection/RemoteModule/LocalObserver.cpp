#include "RemoteConnection/RemoteModule/LocalObserver.hpp"


namespace portaible
{
   


    void LocalObserver::onChannelSubscribed(ChannelData<std::string> channelID)
    {
    }
    
    void LocalObserver::onChannelPublished(ChannelData<std::string> channelID)
    {

    }

    void LocalObserver::onChannelUnsubscribed(ChannelData<std::string> channelID)
    {

    }

    void LocalObserver::onChannelUnpublished(ChannelData<std::string> channelID)
    {

    }

    void LocalObserver::observe(ChannelManager* manager)
    {
        this->subscribedChannel = manager->observeSubscribedChannels(this->makeSubscriber(&LocalObserver::onChannelSubscribed, this));
        this->publishedChannel = manager->observePublishedChannels(this->makeSubscriber(&LocalObserver::onChannelPublished, this));
        this->unsubscribedChannel = manager->observeUnsubscribedChannels(this->makeSubscriber(&LocalObserver::onChannelUnsubscribed, this));
        this->unpublishedChannel = manager->observeUnpublishedChannels(this->makeSubscriber(&LocalObserver::onChannelUnpublished, this));
    }

}