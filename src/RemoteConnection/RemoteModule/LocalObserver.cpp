#include "RemoteConnection/RemoteModule/LocalObserver.hpp"


namespace portaible
{
   


    void LocalObserver::onChannelSubscribed(ChannelData<std::string> channelID)
    {
        Logger::printfln("OnChannelSubscribed %s", channelID->value().c_str());
    }
    
    void LocalObserver::onChannelPublished(ChannelData<std::string> channelID)
    {
        Logger::printfln("OnChannelPublished %s", channelID->value().c_str());
    }

    void LocalObserver::onChannelUnsubscribed(ChannelData<std::string> channelID)
    {
        Logger::printfln("OnChannelUnsubscribed %s", channelID->value().c_str());
    }

    void LocalObserver::onChannelUnpublished(ChannelData<std::string> channelID)
    {
        Logger::printfln("OnChannelUnpublished %s", channelID->value().c_str());
    }

    void LocalObserver::observe(ChannelManager* manager)
    {
        
        this->observedChannelManager = manager;
        this->subscribedChannel = manager->observeSubscribedChannels(this->makeSubscriber(&LocalObserver::onChannelSubscribed, this));
        this->publishedChannel = manager->observePublishedChannels(this->makeSubscriber(&LocalObserver::onChannelPublished, this));
        this->unsubscribedChannel = manager->observeUnsubscribedChannels(this->makeSubscriber(&LocalObserver::onChannelUnsubscribed, this));
        this->unpublishedChannel = manager->observeUnpublishedChannels(this->makeSubscriber(&LocalObserver::onChannelUnpublished, this));
    
        // Since now we get called whenever there is a new channel (un)published or (un)subscribed.

        // We now need to synchronize with the manager.
        // We retrieve the list of published and subscribed channels.
        std::vector<std::string> subscribedChannels;
        std::vector<std::string> publishedChannels;
        manager->getSubscribedAndPublishedChannels(subscribedChannels, publishedChannels);
    }

}