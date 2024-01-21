#include "RemoteConnection/RemoteModule/LocalObserver.hpp"


namespace claid
{
    namespace RemoteConnection
    {
        LocalObserver::LocalObserver() 
        {
  
        }



        void LocalObserver::onChannelSubscribedCallback(ChannelData<std::string> channelID)
        {
            // The sequence ID of the observation channels ((un)subscribed, (un)published)
            // is set to the unique identifier of the module that subscribed or published 
            // a channel.
            size_t moduleIdentifier = channelID->sequenceID;
            Logger::logInfo("Module has subscribed %u subscribed %u %d", moduleIdentifier, this->getUniqueIdentifier(), moduleIdentifier == this->getUniqueIdentifier());
            // We do not want to get notified about subcribing or publishing that we did ourselves.
            if(moduleIdentifier == this->getUniqueIdentifier())
                return;

            this->onChannelSubscribed(channelID->value());
        }
        
        void LocalObserver::onChannelPublishedCallback(ChannelData<std::string> channelID)
        {
            // The sequence ID of the observation channels ((un)subscribed, (un)published)
            // is set to the unique identifier of the module that subscribed or published 
            // a channel.
            size_t moduleIdentifier = channelID->sequenceID;
            // We do not want to get notified about subcribing or publishing that we did ourselves.
            if(moduleIdentifier == this->getUniqueIdentifier())
                return;

            this->onChannelPublished(channelID->value());
        }

        void LocalObserver::onChannelUnsubscribedCallback(ChannelData<std::string> channelID)
        {
            // The sequence ID of the observation channels ((un)subscribed, (un)published)
            // is set to the unique identifier of the module that subscribed or published 
            // a channel.
            size_t moduleIdentifier = channelID->sequenceID;
            // We do not want to get notified about subcribing or publishing that we did ourselves.
            if(moduleIdentifier == this->getUniqueIdentifier())
                return;

            this->onChannelUnsubscribed(channelID->value());
        }

        void LocalObserver::onChannelUnpublishedCallback(ChannelData<std::string> channelID)
        {
            // The sequence ID of the observation channels ((un)subscribed, (un)published)
            // is set to the unique identifier of the module that subscribed or published 
            // a channel.
            size_t moduleIdentifier = channelID->sequenceID;
            // We do not want to get notified about subcribing or publishing that we did ourselves.
            if(moduleIdentifier == this->getUniqueIdentifier())
                return;

            this->onChannelUnpublished(channelID->value());
        }

        void LocalObserver::onChannelSubscribed(const std::string& channelID)
        {
            Logger::logInfo("LocalObserver %ld:OnChannelSubscribed %s %ld", this->getUniqueIdentifier(), channelID.c_str());

            // Send message to remotely connected RunTime that we have a local subscriber for that channel.
            // Therefore, a corresponding RemoteModule running in the other framework can subscribe to that channel.
            //notifyRemoteEntityAboutLocalSubscription(channelID);

           Message message = createChannelUpdateMessage(MessageHeaderChannelUpdate::UpdateType::CHANNEL_SUBSCRIBED, channelID);
           this->sendMessage(message);
        }

        void LocalObserver::onChannelPublished(const std::string& channelID)
        {
            Logger::logInfo("LocalObserver %ld: onChannelPublished %s %ld", this->getUniqueIdentifier(), channelID.c_str());
            Message message = createChannelUpdateMessage(MessageHeaderChannelUpdate::UpdateType::CHANNEL_PUBLISHED, channelID);
            this->sendMessage(message);
        }

        void LocalObserver::onChannelUnsubscribed(const std::string& channelID)
        {
            Logger::logInfo("LocalObserver %ld: onChannelUnsubscribed %s %ld", this->getUniqueIdentifier(), channelID.c_str());
            Message message = createChannelUpdateMessage(MessageHeaderChannelUpdate::UpdateType::CHANNEL_UNSUBSCRIBED, channelID);
            this->sendMessage(message);
        }

        void LocalObserver::onChannelUnpublished(const std::string& channelID)
        {
            Logger::logInfo("LocalObserver %ld: onChannelUnpublished %s", this->getUniqueIdentifier(), channelID.c_str());
            Message message = createChannelUpdateMessage(MessageHeaderChannelUpdate::UpdateType::CHANNEL_UNPUBLISHED, channelID);
            this->sendMessage(message);
        }

        Message LocalObserver::createChannelUpdateMessage(MessageHeaderChannelUpdate::UpdateType type, const std::string& string)
        {
            Message message = Message::CreateMessage<MessageHeaderChannelUpdate, MessageDataString>();
            MessageHeaderChannelUpdate& header = *message.header->as<MessageHeaderChannelUpdate>();
            MessageDataString& data = *message.data->as<MessageDataString>();

            header.updateType = type;
            data.string = string;

            return message;
        }
                

        void LocalObserver::observe(ChannelManager* manager)
        {
            if(this->observedChannelManager != nullptr)
            {
                CLAID_THROW(Exception, "Error in LocalObserver. Observe was called although the LocalObserver already had a ChannelManager set."
                "Was observe called multiple times ? ");
            }
            
            this->observedChannelManager = manager;
            this->subscribedChannel = manager->observeSubscribedChannels(this->makeSubscriber(&LocalObserver::onChannelSubscribedCallback, this), this->getUniqueIdentifier());
            this->publishedChannel = manager->observePublishedChannels(this->makeSubscriber(&LocalObserver::onChannelPublishedCallback, this), this->getUniqueIdentifier());
            this->unsubscribedChannel = manager->observeUnsubscribedChannels(this->makeSubscriber(&LocalObserver::onChannelUnsubscribedCallback, this), this->getUniqueIdentifier());
            this->unpublishedChannel = manager->observeUnpublishedChannels(this->makeSubscriber(&LocalObserver::onChannelUnpublishedCallback, this), this->getUniqueIdentifier());
        
            // Since now we get called whenever there is a new channel (un)published or (un)subscribed.
            // Even if it happens while we are still in this function, we will get notified about it afterwards,
            // as the updates are posted to a channel.
            // When calling manager->getChannelIDs, the ChannelManager locks a mutex, therefore publishing or subscribing channels
            // is not possible. This makes sure that while the function getChannelIDs(...) is called, no channels are (un)published or (un)subscribed.

            // We now need to synchronize with the manager.
            // We retrieve the list of published and subscribed channels and call onChannelPublished or onChannelSubscribed accordingly.
        
            std::vector<std::string> channelIDs;
            manager->getChannelIDs(channelIDs);

            Logger::logInfo("Getting list of channels %d", channelIDs.size());


            for(const std::string& channelID : channelIDs)
            {
                Logger::logInfo("Channel %s", channelID.c_str());

                if(CLAID_RUNTIME->isInHiddenNamespace(channelID))
                {
                    continue;
                }
                if(manager->hasChannelPublisher(channelID))
                {
                    this->onChannelPublished(channelID);
                }
                
                if(manager->hasChannelSubscriber(channelID))
                {  
                    this->onChannelSubscribed(channelID);
                }
            }       
        }

        void LocalObserver::terminate()
        {
            Logger::logInfo("LocalObserver terminate");
            this->subscribedChannel.unsubscribe();
            this->publishedChannel.unsubscribe();
            this->unsubscribedChannel.unsubscribe();
            this->unpublishedChannel.unsubscribe();
        }
    }
}