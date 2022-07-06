#include "Channel.hpp"
#include <map>

namespace portaible
{
    class ChannelManager
    {
        private:
            std::map<std::string, ChannelBase*> typedChannels;

            template<typename T>
            bool canCastChannel(ChannelBase* channel)
            {              
                return channel->getChannelDataTypeUniqueIdentifier() == getDataTypeUniqueIdentifier<T>();
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
                    TypedChannel<T>* newChannel = new TypedChannel<T>(channelID);
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
                    TypedChannel<T>* newChannel = new TypedChannel<T>(channelID);
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
                    TypedChannel<T>* newChannel = new TypedChannel<T>(channelID);
                    this->typedChannels.insert(std::make_pair(channelID, static_cast<ChannelBase*>(newChannel)));
                    return newChannel->publish();
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