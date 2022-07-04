#include "Channel.hpp"
#include <map>

namespace portaible
{
    class ChannelManager
    {
        private:
            std::map<std::string, GlobalChannelBase*> globalChannels;

            template<typename T>
            bool canCastChannel(GlobalChannelBase* channel)
            {              
                return channel->getChannelDataTypeUniqueIdentifier() == getDataTypeUniqueIdentifier<T>();
            }

            template<typename T>
            GlobalChannel<T>* castChannel(GlobalChannelBase* channel)
            {
                return static_cast<GlobalChannel<T>*>(channel);
            }


        public:

            // template<typename T>
            // publish

            template<typename T>
            Channel<T> subscribe(const std::string& channelID)
            {
                auto it = globalChannels.find(channelID);

                if(it != globalChannels.end())
                {
                    GlobalChannelBase* channel = it->second;
                    if(canCastChannel<T>(channel))
                    {
                        // Safely cast GlobalChannelBase to GlobalChannel.
                        GlobalChannel<T>* typedChannel = castChannel<T>(channel);
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
                    GlobalChannel<T>* newChannel = new GlobalChannel<T>(channelID);
                    this->globalChannels.insert(std::make_pair(channelID, static_cast<GlobalChannelBase*>(newChannel)));
                    return newChannel->subscribe();
                }
            }

  

            template<typename T>
            Channel<T> subscribe(const std::string& channelID, ChannelSubscriber<T> channelSubscriber)
            {
                auto it = globalChannels.find(channelID);
                        printf("ChannelManager subscribe %d\n", channelSubscriber.runnableDispatcherThread->id);

                if(it != globalChannels.end())
                {
                    printf("ChannelManager 1\n");
                    GlobalChannelBase* channel = it->second;
                    if(canCastChannel<T>(channel))
                    {
                        // Safely cast GlobalChannelBase to GlobalChannel.
                        GlobalChannel<T>* typedChannel = castChannel<T>(channel);

                        return typedChannel->subscribe(channelSubscriber);
                    }
                    else
                    {
                        PORTAIBLE_THROW(Exception, "Cannot subscribe to channel " << channelID << "! The channel has type " << channel->getChannelDataTypeUniqueIdentifierRTTIString() << ", however a subscription with type" << getDataTypeRTTIString<T>() << " was inquired!" );
                    }
                    
                }
                else
                {
                        printf("ChannelManager subscribe2 %d\n", channelSubscriber.runnableDispatcherThread->id);

                    // Channel not found, create new.
                    GlobalChannel<T>* newChannel = new GlobalChannel<T>(channelID);
                    this->globalChannels.insert(std::make_pair(channelID, static_cast<GlobalChannelBase*>(newChannel)));
                    return newChannel->subscribe(channelSubscriber);
                }
            }

            template<typename T>
            Channel<T> publish(const std::string& channelID)
            {
                auto it = globalChannels.find(channelID);

                if(it != globalChannels.end())
                {
                    GlobalChannelBase* channel = it->second;
                    if(canCastChannel<T>(channel))
                    {
                        // Safely cast GlobalChannelBase to GlobalChannel.
                        GlobalChannel<T>* typedChannel = castChannel<T>(channel);
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
                    GlobalChannel<T>* newChannel = new GlobalChannel<T>(channelID);
                    this->globalChannels.insert(std::make_pair(channelID, static_cast<GlobalChannelBase*>(newChannel)));
                    return newChannel->publish();
                }
            }

            size_t getNumChannels()
            {
                return this->globalChannels.size();
            }

            const std::string& getChannelNameByIndex(size_t id)
            {
                auto it = this->globalChannels.begin();
                std::advance(it, id);
                return it->first;
            }

    };
}