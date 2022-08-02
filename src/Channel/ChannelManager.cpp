#include "Channel/ChannelManager.hpp"

namespace portaible
{

    ChannelManager::ChannelManager()
    {
        // These are visible by channelID (e.g. in subscribe or publish) to others. 
        // They are not part of the channels that can be accessed using publish and subscribe.
        this->onChannelSubscribedChannel = std::shared_ptr<TypedChannel<std::string>>(new TypedChannel<std::string>(this, "ON_CHANNEL_SUBSCRIBED"));
        this->onChannelPublishedChannel = std::shared_ptr<TypedChannel<std::string>>(new TypedChannel<std::string>(this, "ON_CHANNEL_PUBLISHED"));
        this->onChannelUnsubscribedChannel = std::shared_ptr<TypedChannel<std::string>>(new TypedChannel<std::string>(this, "ON_CHANNEL_UNSUBSCRIBED")); 
        this->onChannelUnpublishedChannel = std::shared_ptr<TypedChannel<std::string>>(new TypedChannel<std::string>(this, "ON_CHANNEL_UNPUBLISHED"));
    }

    ChannelManager::~ChannelManager()
    {

    }

    Channel<std::string> ChannelManager::observeSubscribedChannels(ChannelSubscriber<std::string> subscriber)
    {
        return this->onChannelSubscribedChannel->subscribe(subscriber);
    }

    Channel<std::string> ChannelManager::observePublishedChannels(ChannelSubscriber<std::string> subscriber)
    {
        return this->onChannelPublishedChannel->subscribe(subscriber);
    }

    Channel<std::string> ChannelManager::observeUnsubscribedChannels(ChannelSubscriber<std::string> subscriber)
    {
        return this->onChannelUnsubscribedChannel->subscribe(subscriber);
    }
    
    Channel<std::string> ChannelManager::observeUnpublishedChannels(ChannelSubscriber<std::string> subscriber)
    {
        return this->onChannelUnpublishedChannel->subscribe(subscriber);
    }

    void ChannelManager::onChannelSubscribed(const std::string& channelID)
    {
        this->onChannelSubscribedChannel->post(channelID);
    }

    void ChannelManager::onChannelPublished(const std::string& channelID)
    {
        this->onChannelPublishedChannel->post(channelID);
    }

    void ChannelManager::onChannelUnsubscribed(const std::string& channelID)
    {
        this->onChannelUnsubscribedChannel->post(channelID);
    }

    void ChannelManager::onChannelUnpublished(const std::string& channelID)
    {
        this->onChannelUnpublishedChannel->post(channelID);
    }
}