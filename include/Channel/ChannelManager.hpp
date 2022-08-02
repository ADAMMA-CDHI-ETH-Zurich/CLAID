#pragma once

#include "Channel.hpp"
#include "Untyped.hpp"
#include <map>

// TODO: Add Mutex when adding channels or removing? 
namespace portaible
{


    class ChannelManager
    {
        private:
            std::map<std::string, std::shared_ptr<ChannelBase>> typedChannels;
            
            std::mutex channelMapMutex;

            std::shared_ptr<TypedChannel<std::string>> onChannelSubscribedChannel;
            std::shared_ptr<TypedChannel<std::string>> onChannelPublishedChannel;
            std::shared_ptr<TypedChannel<std::string>> onChannelUnsubscribedChannel;
            std::shared_ptr<TypedChannel<std::string>> onChannelUnpublishedChannel;

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

                this->channelMapMutex.lock();
                this->typedChannels.insert(std::make_pair(channelID, std::static_pointer_cast<ChannelBase>(newChannel)));
                this->channelMapMutex.unlock();   

                return newChannel;       
            }

            void onChannelSubscribed(const std::string& channelID);
            void onChannelPublished(const std::string& channelID);
            void onChannelUnsubscribed(const std::string& channelID);
            void onChannelUnpublished(const std::string& channelID);

            // Explicitly forbid copying.
            ChannelManager(const ChannelManager&) = delete;


        public:

            ChannelManager();
            ~ChannelManager();

            // template<typename T>
            // publish

            template<typename T>
            Channel<T> subscribe(const std::string& channelID, bool silent = false)
            {
                Channel<T> returnChannel;
                auto it = typedChannels.find(channelID);

                if(it != typedChannels.end())
                {
                    std::shared_ptr<ChannelBase> channel = it->second;
                    if(canCastChannel<T>(channel))
                    {
                        // Safely cast ChannelBase to TypedChannel.
                        std::shared_ptr<TypedChannel<T>> typedChannel = castChannel<T>(channel);
                        returnChannel = typedChannel->subscribe();                  
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
                    returnChannel = newChannel->subscribe();
                }

                if(!silent)
                {
                    this->onChannelSubscribed(channelID);
                }

                return returnChannel;
            }

  

            template<typename T>
            Channel<T> subscribe(const std::string& channelID, ChannelSubscriber<T> channelSubscriber, bool silent = false)
            {
                Channel<T> returnChannel;
                auto it = typedChannels.find(channelID);

                if(it != typedChannels.end())
                {
                    std::shared_ptr<ChannelBase> channel = it->second;
                    if(canCastChannel<T>(channel))
                    {
                        // Safely cast ChannelBase to TypedChannel.
                        std::shared_ptr<TypedChannel<T>> typedChannel = castChannel<T>(channel);

                        returnChannel = typedChannel->subscribe(channelSubscriber);
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
                    returnChannel = newChannel->subscribe(channelSubscriber);
                }

                if(!silent)
                {
                    this->onChannelSubscribed(channelID);
                }
                return returnChannel;
            }

            template<typename T>
            Channel<T> publish(const std::string& channelID, bool silent = false)
            {
                Channel<T> returnChannel;
                auto it = typedChannels.find(channelID);

                if(it != typedChannels.end())
                {
                    std::shared_ptr<ChannelBase> channel = it->second;
                    if(canCastChannel<T>(channel))
                    {
                        // Safely cast ChannelBase to TypedChannel.
                        std::shared_ptr<TypedChannel<T>> typedChannel = castChannel<T>(channel);
                        returnChannel = typedChannel->publish();
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
                    returnChannel = newChannel->publish();
                }

                if(!silent)
                {
                    this->onChannelPublished(channelID);
                }
                return returnChannel;
            }

            template<typename T>
            void unsubscribe(Channel<T>& channelObject, bool silent = false)
            {
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

                if(!silent)
                {
                    this->onChannelUnsubscribed(channelID);
                }
            }

            template<typename T>
            void unpublish(Channel<T>& channelObject, bool silent = false)
            {
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

                if(!silent)
                {
                    this->onChannelUnpublished(channelID);
                }
            }

            size_t getNumChannels()
            {
                return this->typedChannels.size();
            }

            const std::string& getChannelNameByIndex(size_t id)
            {
                auto it = this->typedChannels.begin();
                std::advance(it, id);
                return it->first;
            }

            Channel<std::string> observeSubscribedChannels(ChannelSubscriber<std::string> subscriber);
            Channel<std::string> observePublishedChannels(ChannelSubscriber<std::string> subscriber);
            Channel<std::string> observeUnsubscribedChannels(ChannelSubscriber<std::string> subscriber);
            Channel<std::string> observeUnpublishedChannels(ChannelSubscriber<std::string> subscriber);

    };
}