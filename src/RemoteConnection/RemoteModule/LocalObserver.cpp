#include "RemoteConnection/RemoteModule/LocalObserver.hpp"


namespace portaible
{
    namespace RemoteConnection
    {
        LocalObserver::LocalObserver(Channel<Message> sendMessageChannel) : sendMessageChannel(sendMessageChannel)
        {

        }



        void LocalObserver::onChannelSubscribedCallback(ChannelData<std::string> channelID)
        {
            // The sequence ID of the observation channels ((un)subscribed, (un)published)
            // is set to the unique identifier of the module that subscribed or published 
            // a channel.
            size_t moduleIdentifier = channelID->sequenceID;
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
            Logger::printfln("OnChannelSubscribed %s", channelID.c_str());
        }

        void LocalObserver::onChannelPublished(const std::string& channelID)
        {
            Logger::printfln("OnChannelPublished %s", channelID.c_str());
        }

        void LocalObserver::onChannelUnsubscribed(const std::string& channelID)
        {
            Logger::printfln("OnChannelUnsubscribed %s", channelID.c_str());
        }

        void LocalObserver::onChannelUnpublished(const std::string& channelID)
        {
            Logger::printfln("OnChannelUnpublished %s", channelID.c_str());
        }

        void LocalObserver::observe(ChannelManager* manager)
        {
            if(this->observedChannelManager != nullptr)
            {
                PORTAIBLE_THROW(Exception, "Error in LocalObserver. Observe was called although the LocalObserver already had a ChannelManager set."
                "Was observe called multiple times ? ");
            }
            
            this->observedChannelManager = manager;
            this->subscribedChannel = manager->observeSubscribedChannels(this->makeSubscriber(&LocalObserver::onChannelSubscribedCallback, this), this->getUniqueIdentifier());
            this->publishedChannel = manager->observePublishedChannels(this->makeSubscriber(&LocalObserver::onChannelPublishedCallback, this), this->getUniqueIdentifier());
            this->unsubscribedChannel = manager->observeUnsubscribedChannels(this->makeSubscriber(&LocalObserver::onChannelUnsubscribedCallback, this), this->getUniqueIdentifier());
            this->unpublishedChannel = manager->observeUnpublishedChannels(this->makeSubscriber(&LocalObserver::onChannelUnpublishedCallback, this), this->getUniqueIdentifier());
        
            // Since now we get called whenever there is a new channel (un)published or (un)subscribed.

            // We now need to synchronize with the manager.
            // We retrieve the list of published and subscribed channels.
        
            std::vector<std::string> channelIDs;
            manager->getChannelIDs(channelIDs);

            for(const std::string& channelID : channelIDs)
            {
                if(manager->hasChannelPublisher(channelID))
                {

                }
                
                if(manager->hasChannelSubscriber(channelID))
                {
                    // Send message to remotely connected RunTime that we have a local subscriber for that channel.
                    // Therefore, a corresponding RemoteModule running in the other framework can subscribe to that channel.
                    //notifyRemoteEntityAboutLocalSubscription(channelID);
                }
            }       
        }
    }
}