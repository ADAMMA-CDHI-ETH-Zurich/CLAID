#include "Channel/ChannelManager.hpp"
#include "RunTime/RunTime.hpp"
namespace claid
{
    const std::string ChannelManager::ON_CHANNEL_SUBSCRIBED_CHANNEL = "ON_CHANNEL_SUBSCRIBED";
    const std::string ChannelManager::ON_CHANNEL_PUBLISHED_CHANNEL = "ON_CHANNEL_PUBLISHED";
    const std::string ChannelManager::ON_CHANNEL_UNSUBSCRIBED_CHANNEL = "ON_CHANNEL_UNSUBSCRIBED";
    const std::string ChannelManager::ON_CHANNEL_UNPUBLISHED_CHANNEL = "ON_CHANNEL_UNPUBLISHED";

    ChannelManager::ChannelManager()
    {
        // These are visible by channelID (e.g. in subscribe or publish) to others. 
        // They are not part of the channels that can be accessed using publish and subscribe.
        this->onChannelSubscribedChannel = std::shared_ptr<TypedChannel<std::string>>(new TypedChannel<std::string>(this, ON_CHANNEL_SUBSCRIBED_CHANNEL));
        this->onChannelPublishedChannel = std::shared_ptr<TypedChannel<std::string>>(new TypedChannel<std::string>(this, ON_CHANNEL_PUBLISHED_CHANNEL));
        this->onChannelUnsubscribedChannel = std::shared_ptr<TypedChannel<std::string>>(new TypedChannel<std::string>(this, ON_CHANNEL_UNSUBSCRIBED_CHANNEL)); 
        this->onChannelUnpublishedChannel = std::shared_ptr<TypedChannel<std::string>>(new TypedChannel<std::string>(this, ON_CHANNEL_UNPUBLISHED_CHANNEL));
    }

    ChannelManager::~ChannelManager()
    {

    }

    size_t ChannelManager::getNumChannels()
    {
        return this->typedChannels.size();
    }

    const std::string& ChannelManager::getChannelNameByIndex(size_t id)
    {
        auto it = this->typedChannels.begin();
        std::advance(it, id);
        return it->first;
    }

    void ChannelManager::getChannelIDs(std::vector<std::string>& channelIDs)
    {
        std::unique_lock<std::mutex> lock(this->channelMutex);
        channelIDs.clear();

        for(auto it : this->typedChannels)
        {
            channelIDs.push_back(it.first);
        }
    }

    bool ChannelManager::hasChannelPublisher(const std::string& channelID)
    {
        std::unique_lock<std::mutex> lock(this->channelMutex);
        auto it = this->typedChannels.find(channelID);
        
        if(it == typedChannels.end())
        {
            return false;
        }

        return it->second->getNumPublishers() > 0;
    }

    bool ChannelManager::hasChannelSubscriber(const std::string& channelID)
    {
        std::unique_lock<std::mutex> lock(this->channelMutex);
        auto it = this->typedChannels.find(channelID);
        
        if(it == typedChannels.end())
        {
            return false;
        }

        return it->second->getNumSubscribers() > 0;
    }

    Channel<std::string> ChannelManager::observeSubscribedChannels(ChannelSubscriber<std::string> subscriber, uint64_t observerUniqueModuleID)
    {
        return this->onChannelSubscribedChannel->subscribe(subscriber, observerUniqueModuleID);
    }

    Channel<std::string> ChannelManager::observePublishedChannels(ChannelSubscriber<std::string> subscriber, uint64_t observerUniqueModuleID)
    {
        return this->onChannelPublishedChannel->subscribe(subscriber, observerUniqueModuleID);
    }

    Channel<std::string> ChannelManager::observeUnsubscribedChannels(ChannelSubscriber<std::string> subscriber, uint64_t observerUniqueModuleID)
    {
        return this->onChannelUnsubscribedChannel->subscribe(subscriber, observerUniqueModuleID);
    }
    
    Channel<std::string> ChannelManager::observeUnpublishedChannels(ChannelSubscriber<std::string> subscriber, uint64_t observerUniqueModuleID)
    {
        return this->onChannelUnpublishedChannel->subscribe(subscriber, observerUniqueModuleID);
    }

    void ChannelManager::onChannelSubscribed(const std::string& channelID, uint64_t uniqueModuleID)
    {
        this->onChannelSubscribedChannel->post(channelID, Time::now(), uniqueModuleID);
    }

    void ChannelManager::onChannelPublished(const std::string& channelID, uint64_t uniqueModuleID)
    {
        this->onChannelPublishedChannel->post(channelID, Time::now(), uniqueModuleID);
    }

    void ChannelManager::onChannelUnsubscribed(const std::string& channelID, uint64_t uniqueModuleID)
    {
        this->onChannelUnsubscribedChannel->post(channelID, Time::now(), uniqueModuleID);
    }

    void ChannelManager::onChannelUnpublished(const std::string& channelID, uint64_t uniqueModuleID)
    {
        this->onChannelUnpublishedChannel->post(channelID, Time::now(), uniqueModuleID);
    }

    bool ChannelManager::isInHiddenNamespace(const std::string& channelID) const
    {
        return CLAID_RUNTIME->isInHiddenNamespace(channelID);
    }


}