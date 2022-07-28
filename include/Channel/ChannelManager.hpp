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
            std::map<std::string, ChannelBase*> typedChannels;
            

            
            template<typename T>
            typename std::enable_if<!std::is_same<T, Untyped>::value,bool>::type
            canCastChannel(ChannelBase* channel)
            {              
                return channel->getChannelDataTypeUniqueIdentifier() == getDataTypeUniqueIdentifier<T>();
            }

            template<typename T>
            typename std::enable_if<std::is_same<T, Untyped>::value,bool>::type
            canCastChannel(ChannelBase* channel)
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
            TypedChannel<T>* castChannel(ChannelBase* channel)
            {
                return static_cast<TypedChannel<T>*>(channel);
            }


        public:

            // template<typename T>
            // publish

            template<typename T>
            Channel<T> subscribe(const std::string& channelID)
            {
                auto it = typedChannels.find(channelID);

                if(it != typedChannels.end())
                {
                    ChannelBase* channel = it->second;
                    if(canCastChannel<T>(channel))
                    {
                        // Safely cast ChannelBase to TypedChannel.
                        TypedChannel<T>* typedChannel = castChannel<T>(channel);
                        return typedChannel->subscribe();
                    }
                    else
                    {
                        PORTAIBLE_THROW(Exception, "Cannot subscribe to channel " << channelID << "! The channel has type " << channel->getChannelDataTypeUniqueIdentifierRTTIString() << ", however a subscription with type" << getDataTypeRTTIString<T>() << " was inquired!" );
                    }
                    
                }
                else
                {
                    // Channel not found, create new.
                    TypedChannel<T>* newChannel = new TypedChannel<T>(this, channelID);
                    this->typedChannels.insert(std::make_pair(channelID, static_cast<ChannelBase*>(newChannel)));
                    return newChannel->subscribe();
                }
            }

  

            template<typename T>
            Channel<T> subscribe(const std::string& channelID, ChannelSubscriber<T> channelSubscriber)
            {
                auto it = typedChannels.find(channelID);

                if(it != typedChannels.end())
                {
                    ChannelBase* channel = it->second;
                    if(canCastChannel<T>(channel))
                    {
                        // Safely cast ChannelBase to TypedChannel.
                        TypedChannel<T>* typedChannel = castChannel<T>(channel);

                        return typedChannel->subscribe(channelSubscriber);
                    }
                    else
                    {
                        PORTAIBLE_THROW(Exception, "Cannot subscribe to channel " << channelID << "! The channel has type " << channel->getChannelDataTypeUniqueIdentifierRTTIString() << ", however a subscription with type" << getDataTypeRTTIString<T>() << " was inquired!" );
                    }
                    
                }
                else
                {

                    // Channel not found, create new.
                    TypedChannel<T>* newChannel = new TypedChannel<T>(this, channelID);
                    this->typedChannels.insert(std::make_pair(channelID, static_cast<ChannelBase*>(newChannel)));
                    return newChannel->subscribe(channelSubscriber);
                }
            }

            template<typename T>
            Channel<T> publish(const std::string& channelID)
            {
                auto it = typedChannels.find(channelID);

                if(it != typedChannels.end())
                {
                    ChannelBase* channel = it->second;
                    if(canCastChannel<T>(channel))
                    {
                        // Safely cast ChannelBase to TypedChannel.
                        TypedChannel<T>* typedChannel = castChannel<T>(channel);
                        return typedChannel->publish();
                    }
                    else
                    {
                        PORTAIBLE_THROW(Exception, "Cannot publish channel " << channelID << "! The channel already exists and has type " << channel->getChannelDataTypeUniqueIdentifierRTTIString() << ", however, publishing of a channel with the same channel ID but type" << getDataTypeRTTIString<T>() << " was requested!" );
                    }
                    
                }
                else
                {
                    // Channel not found, create new.
                    TypedChannel<T>* newChannel = new TypedChannel<T>(this, channelID);
                    this->typedChannels.insert(std::make_pair(channelID, static_cast<ChannelBase*>(newChannel)));
                    return newChannel->publish();
                }
            }

            template<typename T>
            void unsubscribe(Channel<T>& channelObject)
            {
                const std::string channelID = channelObject.getChannelID();

                auto it = typedChannels.find(channelID);

                if(it != typedChannels.end())
                {
                    ChannelBase* channel = it->second;
                    if(canCastChannel<T>(channel))
                    {
                        TypedChannel<T>* typedChannel = castChannel<T>(channel);
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
            }

            template<typename T>
            void unpublish(Channel<T>& channelObject)
            {
                const std::string channelID = channelObject.getChannelID();

                auto it = typedChannels.find(channelID);

                if(it != typedChannels.end())
                {
                    ChannelBase* channel = it->second;
                    if(canCastChannel<T>(channel))
                    {
                        TypedChannel<T>* typedChannel = castChannel<T>(channel);
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

            

    };
}