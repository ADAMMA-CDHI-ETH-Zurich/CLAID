#include "RemoteConnection/RemoteModule/RemoteObserver.hpp"
#include "RemoteConnection/Message/MessageData/MessageDataBinary.hpp"

namespace portaible
{
    namespace RemoteConnection
    {
        RemoteObserver::RemoteObserver(ChannelManager* globalChannelManager, Channel<Message> sendMessageChannel) : globalChannelManager(globalChannelManager), sendMessageChannel(sendMessageChannel)
        {

        }

        ChannelSubscriber<Message> RemoteObserver::getSubscriberForReceptionOfMessages()
        {
            return this->makeSubscriber(&RemoteObserver::onMessageReceived, this);
        }

        void RemoteObserver::onMessageReceived(ChannelData<Message> message)
        {
            // Get a copy of the message (data and header of message are shared_ptr,
            // so no overhead), because message->value() is const.
      //      Message copy = message->value();
            Logger::printfln("OnMessageReceived");
            const Message& messageRef = message->value();

            // Calls onChannelUpdate if message.header is MessageHeaderChannelUpdate and message.data is MessageDataString.
            // Throws exception, if header types match (message.header is MessageHeaderChannelUpdate), but data type does not (message.data is not MessageDataString).
            if(callFunctionIfSignatureMatchesThrowExceptionIfWrongData<MessageHeaderChannelUpdate, MessageDataString>(messageRef, &RemoteObserver::onChannelUpdateMessage, this)) return;
            if(callFunctionIfSignatureMatchesThrowExceptionIfWrongData<MessageHeaderChannelData, MessageDataBinary>(messageRef, &RemoteObserver::onChannelDataMessage, this)) return;
           
           // if(callFunctionIfSignatureMatchesThrowExceptionIfWrongData<MessageHeaderChannelData, MessageDataBinary>(messageRef, &RemoteObserver::onChannelData, this)) return;


            PORTAIBLE_THROW(Exception, "Error! RemoteObserver received message with unexpected header \"" << messageRef.header->getClassName() << "\".");

        }    

        void RemoteObserver::onChannelUpdateMessage(const MessageHeaderChannelUpdate& header, const MessageDataString& data)
        {
            typedef MessageHeaderChannelUpdate::UpdateType UpdateType;


            const UpdateType type = header.updateType;
            const std::string& channelID = data.string;

            switch(type)
            {
                case UpdateType::CHANNEL_PUBLISHED:
                {
                    this->onChannelPublished(channelID);
                }
                break;

                case UpdateType::CHANNEL_SUBSCRIBED:
                {
                    this->onChannelSubscribed(channelID);
                }
                break;

                case UpdateType::CHANNEL_UNPUBLISHED:
                {
                    this->onChannelUnpublished(channelID);
                }
                break;

                case UpdateType::CHANNEL_UNSUBSCRIBED:
                {
                    this->onChannelUnsubscribed(channelID);
                }
                break;

                default:
                {
                    PORTAIBLE_THROW(Exception, "Error, received message of type MessageHeaderChannelUpdate with unsupported Update type " << __FILE__ << " " << __LINE__ << "\n");
                }
                break;
            }
        }

        void RemoteObserver::onChannelDataMessage(const MessageHeaderChannelData& header, const MessageDataBinary& data)
        {
            // onChannelDataReceivedFromRemoteRunTime
        }

        // Some module in the remote RunTime subscribed to a channel.
        // Thus, whenever data is published to that channel in the local RunTime, we need
        // to send the data to the remote RunTime.
        void RemoteObserver::onChannelSubscribed(const std::string& channelID)
        {
            // See comments in header on subscribedChannelsWithCallback
            auto it = this->subscribedChannelsWithCallback.find(channelID);

            if(it == this->subscribedChannelsWithCallback.end())
            {
                // Cannot pass reference to bind.
                std::string channelIDCopy = channelID;
                std::function<void (ChannelData<Untyped>)> function = std::bind(&RemoteObserver::onNewLocalDataInChannelThatRemoteRunTimeHasSubscribedTo, this, channelIDCopy, std::placeholders::_1);
                Channel<Untyped> channel = this->subscribe<Untyped>(channelID, function);
                this->subscribedChannelsWithCallback.insert(std::make_pair(channelID, channel));
            }
            else
            {
                Channel<Untyped> channel = this->subscribe<Untyped>(channelID);
                this->subscribedChannels.insert(std::make_pair(channelID, channel));
            }

        }

        void RemoteObserver::onChannelPublished(const std::string& channelID)
        {
            Channel<Untyped> channel = this->publish<Untyped>(channelID);
            this->publishedChannels.insert(std::make_pair(channelID, channel));
        }

        void RemoteObserver::onChannelUnsubscribed(const std::string& channelID)
        {
            // First, check if we have subscribed to that channel without callback.
            auto it = this->subscribedChannels.find(channelID);

            if(it != this->subscribedChannels.end())
            {
                // If yes, unsubscribe one channel and remove it from the list.
                it->second.unsubscribe();
                this->subscribedChannels.erase(it);
            }
            else
            {
                // If no, that means that we *MUST* have only *ONE* subscriber left,
                // the one that we registered a callback for.
                // Thus, we need to unsuscribe of that one aswell.
                auto it2 = this->subscribedChannelsWithCallback.find(channelID);

                if(it2 != this->subscribedChannelsWithCallback.end())
                {
                    it2->second.unsubscribe();
                    this->subscribedChannelsWithCallback.erase(it2);
                }
                else
                {
                    // ERROR! No subscriber left, we unsubscribed more than we have subscribed.
                    // Local and remote RunTime out of sync? Severe error.
                    PORTAIBLE_THROW(Exception, "Error in RemoteObserver: Received message that indicates we shall unsubscribe from channel with ID \"" << channelID << "\", "
                    << "however we have no subscriber left for that channel. Therefore, we unsubscribed more than we subscribed. Somehow, we have to have missed a subscription in " 
                    << "the remotely running RunTime, therefore the local and remote RunTime probably are out of sync.");
                }
            }
        }

        void RemoteObserver::onChannelUnpublished(const std::string& channelID)
        {
            // Check if we have published that channel.
            auto it = this->publishedChannels.find(channelID);

            if(it != this->publishedChannels.end())
            {
                // If yes, unpublish one channel and remove it from the list.
                it->second.unpublish();
                this->publishedChannels.erase(it);
            }
            else
            {
                // Error, more unpublish channel more often than publish.
                // This should not happen. That would mean in the remote RunTime, channel was unpublished more than
                // published. Possibily the local and remote RunTime are out of sync.

                // ERROR! No publisher left, we unpublished more than we have published.
                // Local and remote RunTime out of sync? Severe error.
                PORTAIBLE_THROW(Exception, "Error in RemoteObserver: Received message that indicates we shall unpublish channel with ID \"" << channelID << "\", "
                << "however we have no publisher left for that channel. Therefore, we unpublished more than we published. Somehow, we have to have missed a publish in " 
                << "the remotely running RunTime, therefore the local and remote RunTime probably are out of sync.");
            }
        }

        void RemoteObserver::onNewLocalDataInChannelThatRemoteRunTimeHasSubscribedTo(std::string channelID, ChannelData<Untyped> data)
        {
            // Need to send to the remote RunTime
            // Get tagged binary data
            TaggedData<BinaryData> taggedBinaryData;
            MessageDataBinary messageDataBinary;

            Message message = Message::CreateMessage<MessageHeaderChannelData, MessageDataBinary>();
    
            message.header->as<MessageHeaderChannelData>()->targetChannel = channelID;
            // Set will serialize TaggedData<BinaryData>.
            // TaggedData holds the header (timestamp, sequenceID) and the binary data.
            // Thus, timestamp and sequenceID will be serialized, the binary data will be copied into a bigger
            // binary data buffer that contains timestamp, sequenceID and the binary data itself.
            message.data->as<MessageDataBinary>()->set<TaggedData<BinaryData>>(taggedBinaryData);
            this->sendMessage(message);

            
            // // Check if we have published that channel.
            // auto it = this->publishedChannels.find(channelID);

            // if(it != this->publishedChannels.end())
            // {
            //     //channelData.readBinaryData();
            //     //it->second.writeBinaryData();
            // }
            // else
            // {
            //     // Error, more unpublish channel more often than publish.
            //     // This should not happen. That would mean in the remote RunTime, channel was unpublished more than
            //     // published. Possibily the local and remote RunTime are out of sync.

            //     // ERROR! No publisher left, we unpublished more than we have published.
            //     // Local and remote RunTime out of sync? Severe error.
            //     PORTAIBLE_THROW(Exception, "Error in RemoteObserver: Received message that indicates we shall unpublish channel with ID \"" << channelID << "\", "
            //     << "however we have no publisher left for that channel. Therefore, we unpublished more than we published. Somehow, we have to have missed a publish in " 
            //     << "the remotely running RunTime, therefore the local and remote RunTime probably are out of sync.");
            // }
        }

        void RemoteObserver::sendMessage(const Message& message)
        {
            this->sendMessageChannel.post(message);
        }
                
    }
}


       

