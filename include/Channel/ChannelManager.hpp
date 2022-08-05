#pragma once

#include "Channel.hpp"
#include "Untyped.hpp"
#include <map>

namespace portaible
{


    class ChannelManager
    {
        private:
            std::map<std::string, std::shared_ptr<ChannelBase>> typedChannels;
            
            std::mutex channelMutex;

            std::shared_ptr<TypedChannel<std::string>> onChannelSubscribedChannel;
            std::shared_ptr<TypedChannel<std::string>> onChannelPublishedChannel;
            std::shared_ptr<TypedChannel<std::string>> onChannelUnsubscribedChannel;
            std::shared_ptr<TypedChannel<std::string>> onChannelUnpublishedChannel;

            bool accessToChannelsBlocked = false;

            template<typename T>
            typename std::enable_if<!std::is_same<T, Untyped>::value,bool>::type
            canCastChannel(std::shared_ptr<ChannelBase> channel)
            {              
                return channel->getChannelDataTypeUniqueIdentifier() == getDataTypeUniqueIdentifier<T>();
            }

            template<typename T>
            typename std::enable_if<std::is_same<T, Untyped>::value,bool>::type
            canCastChannel(std::shared_ptr<ChannelBase> channel)
            {              
                // Every channel can be casted to Untyped channel.
                // Why, you ask? Even that TypedChannel<T> does not inherit from Channel<Untyped>?
                // Well, that's because TypedChannel<T> does not introduce any new member variables
                // compared to ChannelBase. In other words, TypedChannel<T> extends ChannelBase ONLY
                // by FUNCTIONS and NOT by variables.
                // Thus, TypedChannel<T> merely acts as a "view" to the underlying data.
                
                // But wait, doesn't that mean we could cast every TypedChannel<T> to every other
                // TypedChannel<T>? Well yes, theoretically. However, when you'd call post you might
                // end up inserting wrong data into the ChannelBuffer. That's why for 
                // TypedChannel<Untyped> (and Channel<Untyped>), the post function is disabled.
                return true;
            }

            template<typename T>
            std::shared_ptr<TypedChannel<T>> castChannel(std::shared_ptr<ChannelBase> channel)
            {
                return std::static_pointer_cast<TypedChannel<T>>(channel);
            }

     

            template<typename T>
            std::shared_ptr<TypedChannel<T>> registerNewChannel(const std::string& channelID)
            {
                std::shared_ptr<TypedChannel<T>> newChannel = std::shared_ptr<TypedChannel<T>>(new TypedChannel<T>(this, channelID));

                // Dont lock the mutex here, as it already get's locked in publish or subscribe functions.
                //std::unique_lock<std::mutex> lock(this->channelMutex);
                this->typedChannels.insert(std::make_pair(channelID, std::static_pointer_cast<ChannelBase>(newChannel)));

                return newChannel;       
            }

            void onChannelSubscribed(const std::string& channelID, uint64_t uniqueModuleID);
            void onChannelPublished(const std::string& channelID, uint64_t uniqueModuleID);
            void onChannelUnsubscribed(const std::string& channelID, uint64_t uniqueModuleID);
            void onChannelUnpublished(const std::string& channelID, uint64_t uniqueModuleID);

            // Explicitly forbid copying.
            ChannelManager(const ChannelManager&) = delete;

 
        public:

            ChannelManager();
            ~ChannelManager();

            // template<typename T>
            // publish

            
            // uniqueModuleID can be used to identify which module has subscribed / published to a channel.
            // It is only used for the onChannel(Un)Subscribed, onChannel(Un)Published functions.
            // So far only has a use for the RemoteModule
            template<typename T>
            Channel<T> subscribe(const std::string& channelID, uint64_t uniqueModuleID)
            {
                std::unique_lock<std::mutex> lock(this->channelMutex);

                Channel<T> returnChannel;
                auto it = typedChannels.find(channelID);

                if(it != typedChannels.end())
                {
                    std::shared_ptr<ChannelBase> channel = it->second;
                    if(canCastChannel<T>(channel))
                    {
                        // Safely cast ChannelBase to TypedChannel.
                        std::shared_ptr<TypedChannel<T>> typedChannel = castChannel<T>(channel);
                        returnChannel = typedChannel->subscribe(uniqueModuleID);                  
                    }
                    else
                    {
                        PORTAIBLE_THROW(Exception, "Cannot subscribe to channel " << channelID << "! The channel has type " << channel->getChannelDataTypeUniqueIdentifierRTTIString() << ", however a subscription with type" << getDataTypeRTTIString<T>() << " was inquired!" );
                    }
                    
                }
                else
                {
                    // Channel not found, create new.
                    TypedChannel<T>* newChannel = registerNewChannel<T>(channelID);   
                    returnChannel = newChannel->subscribe(uniqueModuleID);
                }

                this->onChannelSubscribed(channelID, uniqueModuleID);
                return returnChannel;
            }

  

            template<typename T>
            Channel<T> subscribe(const std::string& channelID, ChannelSubscriber<T> channelSubscriber, uint64_t uniqueModuleID)
            {
                std::unique_lock<std::mutex> lock(this->channelMutex);


                Channel<T> returnChannel;
                auto it = typedChannels.find(channelID);

                if(it != typedChannels.end())
                {
                    std::shared_ptr<ChannelBase> channel = it->second;
                    if(canCastChannel<T>(channel))
                    {
                        // Safely cast ChannelBase to TypedChannel.
                        std::shared_ptr<TypedChannel<T>> typedChannel = castChannel<T>(channel);

                        returnChannel = typedChannel->subscribe(channelSubscriber, uniqueModuleID);
                    }
                    else
                    {
                        PORTAIBLE_THROW(Exception, "Cannot subscribe to channel " << channelID << "! The channel has type " << channel->getChannelDataTypeUniqueIdentifierRTTIString() << ", however a subscription with type" << getDataTypeRTTIString<T>() << " was inquired!" );
                    }
                    
                }
                else
                {

                    // Channel not found, create new.
                    std::shared_ptr<TypedChannel<T>> newChannel = registerNewChannel<T>(channelID);
                    returnChannel = newChannel->subscribe(channelSubscriber, uniqueModuleID);
                }

               
                this->onChannelSubscribed(channelID, uniqueModuleID);
                return returnChannel;
            }

            template<typename T>
            Channel<T> publish(const std::string& channelID, uint64_t uniqueModuleID)
            {
                std::unique_lock<std::mutex> lock(this->channelMutex);

                Channel<T> returnChannel;
                auto it = typedChannels.find(channelID);

                if(it != typedChannels.end())
                {
                    std::shared_ptr<ChannelBase> channel = it->second;
                    if(canCastChannel<T>(channel))
                    {
                        // Safely cast ChannelBase to TypedChannel.
                        std::shared_ptr<TypedChannel<T>> typedChannel = castChannel<T>(channel);
                        returnChannel = typedChannel->publish(uniqueModuleID);
                    }
                    else
                    {
                        PORTAIBLE_THROW(Exception, "Cannot publish channel " << channelID << "! The channel already exists and has type " << channel->getChannelDataTypeUniqueIdentifierRTTIString() << ", however, publishing of a channel with the same channel ID but type" << getDataTypeRTTIString<T>() << " was requested!" );
                    }
                    
                }
                else
                {
                    // Channel not found, create new.
                    std::shared_ptr<TypedChannel<T>> newChannel = registerNewChannel<T>(channelID);
                    returnChannel = newChannel->publish(uniqueModuleID);
                }


                this->onChannelPublished(channelID, uniqueModuleID);
                return returnChannel;
            }

            template<typename T>
            void unsubscribe(Channel<T>& channelObject)
            {
                std::unique_lock<std::mutex> lock(this->channelMutex);

                const std::string channelID = channelObject.getChannelID();

                auto it = typedChannels.find(channelID);

                if(it != typedChannels.end())
                {
                    std::shared_ptr<ChannelBase> channel = it->second;
                    if(canCastChannel<T>(channel))
                    {
                        std::shared_ptr<TypedChannel<T>> typedChannel = castChannel<T>(channel);
                        typedChannel->unsubscribe(channelObject);
                    }
                    else
                    {
                        PORTAIBLE_THROW(Exception, "Error, cannot unsubscribe from channel with ID" << channelID << 
                            "The type of the channel (" << channel->getChannelDataTypeUniqueIdentifierRTTIString() << ") cannot be cast to " << getDataTypeRTTIString<T>() << ".");
                    }
                }
                else
                {
                    PORTAIBLE_THROW(Exception, "Error, unsubscribe was called on a channel with channel ID " << channelID << ", which does not exist. This should never happen and most likely is a programming mistake");
                }

                this->onChannelUnsubscribed(channelID, channelObject.getPublisherSubscriberUniqueIdentifier());
            }

            template<typename T>
            void unpublish(Channel<T>& channelObject)
            {
                std::unique_lock<std::mutex> lock(this->channelMutex);

                const std::string channelID = channelObject.getChannelID();

                auto it = typedChannels.find(channelID);

                if(it != typedChannels.end())
                {
                    std::shared_ptr<ChannelBase> channel = it->second;
                    if(canCastChannel<T>(channel))
                    {
                        std::shared_ptr<TypedChannel<T>> typedChannel = castChannel<T>(channel);
                        typedChannel->unpublish(channelObject);
                    }
                    else
                    {
                        PORTAIBLE_THROW(Exception, "Error, cannot unpublish channel with ID" << channelID << 
                            "The type of the channel (" << channel->getChannelDataTypeUniqueIdentifierRTTIString() << ") cannot be cast to " << getDataTypeRTTIString<T>() << ".");
                    }
                }
                else
                {
                    PORTAIBLE_THROW(Exception, "Error, unpublish was called on a channel with channel ID " << channelID << ", which does not exist. This should never happen and most likely is a programming mistake");
                }

                this->onChannelUnpublished(channelID, channelObject.getPublisherSubscriberUniqueIdentifier());
            }

            size_t getNumChannels();

            const std::string& getChannelNameByIndex(size_t id);


            bool hasChannelPublisher(const std::string& channelID);
            bool hasChannelSubscriber(const std::string& channelID);

                 
            Channel<std::string> observeSubscribedChannels(ChannelSubscriber<std::string> subscriber, uint64_t observerUniqueModuleID);
            Channel<std::string> observePublishedChannels(ChannelSubscriber<std::string> subscriber, uint64_t observerUniqueModuleID);
            Channel<std::string> observeUnsubscribedChannels(ChannelSubscriber<std::string> subscriber, uint64_t observerUniqueModuleID);
            Channel<std::string> observeUnpublishedChannels(ChannelSubscriber<std::string> subscriber, uint64_t observerUniqueModuleID);

            void getChannelIDs(std::vector<std::string>& channelIDs);

    };
}

