#include "RemoteConnection/RemoteModule/RemoteObserver.hpp"
#include "RemoteConnection/Message/MessageData/MessageDataBinary.hpp"
#include "RemoteConnection/Message/MessageHeader/TestMessage.hpp"
#include "RemoteConnection/Error/ErrorRemoteRuntimeOutOfSync.hpp"
#include "RemoteConnection/Error/ErrorConnectionTimeout.hpp"
#include "Utilities/Time.hpp"
namespace claid
{
    namespace RemoteConnection
    {
        const std::string RemoteObserver::IS_DATA_RECEIVED_FROM_REMOTE_TAG = "REMOTE_OBSERVER_IS_DATA_RECEIVED_FROM_REMOTE";
        const uint32_t RemoteObserver::KEEP_ALIVE_INTERVAL_MILLISECONDS = 3000;

        RemoteObserver::RemoteObserver(ChannelManager* globalChannelManager) : globalChannelManager(globalChannelManager)
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
            if(messageRef.header->is<MessageHeaderChannelUpdate>())
            {
                Logger::printfln("Header is MessageHeaderChannelUpdate");

            }
            else if(messageRef.header->is<MessageHeaderChannelData>())
            {
                Logger::printfln("Header is MessageHeaderChannelData");
            }

            // Calls onChannelUpdate if message.header is MessageHeaderChannelUpdate and message.data is MessageDataString.
            // Throws exception, if header types match (message.header is MessageHeaderChannelUpdate), but data type does not (message.data is not MessageDataString).
            if(callFunctionIfSignatureMatchesThrowExceptionIfWrongData<MessageHeaderChannelUpdate, MessageDataString>(messageRef, &RemoteObserver::onChannelUpdateMessage, this)) return;
            if(callFunctionIfSignatureMatchesThrowExceptionIfWrongData<MessageHeaderChannelData, MessageDataBinary>(messageRef, &RemoteObserver::onChannelDataMessage, this)) return;
            if(callFunctionIfSignatureMatchesThrowExceptionIfWrongData<MessageHeaderKeepAlive, MessageDataEmpty>(messageRef, &RemoteObserver::onKeepAliveMessage, this)) return;
            if(callFunctionIfSignatureMatchesThrowExceptionIfWrongData<MessageHeaderKeepAliveResponse, MessageDataEmpty>(messageRef, &RemoteObserver::onKeepAliveResponse, this)) return;

            if(messageRef.header->is<TestMessage>())
            {
                return;
            }
           // if(callFunctionIfSignatureMatchesThrowExceptionIfWrongData<MessageHeaderChannelData, MessageDataBinary>(messageRef, &RemoteObserver::onChannelData, this)) return;


            CLAID_THROW(Exception, "Error! RemoteObserver received message with unexpected header \"" << ClassFactory::getInstance()->getClassNameOfObject(messageRef) << "\".");

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
                    CLAID_THROW(Exception, "Error, received message of type MessageHeaderChannelUpdate with unsupported Update type " << __FILE__ << " " << __LINE__ << "\n");
                }
                break;
            }
        }

        void RemoteObserver::onChannelDataMessage(const MessageHeaderChannelData& header, const MessageDataBinary& data)
        {

            // we know what we are doing... hopefully
            // Deserialization does not alter the original binary data.
            // However, our reflection system does not support const types.
            const MessageDataBinary* constPtr = &data;
            MessageDataBinary* nonConstPtr = const_cast<MessageDataBinary*>(constPtr);
            
            std::shared_ptr<BinaryData> binaryData(new BinaryData);
            nonConstPtr->getBinaryData(*binaryData.get());


            TaggedData<BinaryData> taggedBinaryData(binaryData, header.header.timestamp, header.header.sequenceID);
            Logger::printfln("Received timestamp %d", header.header.timestamp.toUnixTimestamp());
            this->onChannelDataReceivedFromRemoteRunTime(header.targetChannel, taggedBinaryData);
        }


        // Some module in the remote RunTime subscribed to a channel.
        // Thus, whenever data is published to that channel in the local RunTime, we need
        // to send the data to the remote RunTime.
        void RemoteObserver::onChannelSubscribed(const std::string& channelID)
        {
            Logger::printfln("RemoteModule:: onChannelSubscribed %s", channelID.c_str());
            // See comments in header on subscribedChannelsWithCallback
            auto it = this->subscribedChannelsWithCallback.find(channelID);

            if(it == this->subscribedChannelsWithCallback.end())
            {
                Logger::printfln("Subscribing with callback");
                // Cannot pass reference to bind.
                std::string channelIDCopy = channelID;
                std::function<void (ChannelData<Untyped>)> function = std::bind(&RemoteObserver::onNewLocalDataInChannelThatRemoteRunTimeHasSubscribedTo, this, channelIDCopy, std::placeholders::_1);
                Channel<Untyped> channel = this->subscribe<Untyped>(channelID, function);
                this->subscribedChannelsWithCallback.insert(std::make_pair(channelID, channel));
            }
            else
            {
                Logger::printfln("Subscribing without callback");
                Channel<Untyped> channel = this->subscribe<Untyped>(channelID);
                this->subscribedChannels.insert(std::make_pair(channelID, channel));
            }

        }

        void RemoteObserver::onChannelPublished(const std::string& channelID)
        {
            Logger::printfln("RemoteObserver: onChannelPublished %s", channelID.c_str());
            Channel<Untyped> channel = this->publish<Untyped>(channelID);
            this->publishedChannels.insert(std::make_pair(channelID, channel));
        }

        void RemoteObserver::onChannelUnsubscribed(const std::string& channelID)
        {
            Logger::printfln("RemoteObserver: onChannelUnsubscribed %s", channelID.c_str());
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
                    // Can happen, if we miss a message for example (e.g., temporary loss of network).
                    this->postError<ErrorRemoteRuntimeOutOfSync>();
                    // ERROR! No subscriber left, we unsubscribed more than we have subscribed.
                    // Local and remote RunTime out of sync? Severe error.
                    /*CLAID_THROW(Exception, "Error in RemoteObserver: Received message that indicates we shall unsubscribe from channel with ID \"" << channelID << "\", "
                    << "however we have no subscriber left for that channel. Therefore, we unsubscribed more than we subscribed. Somehow, we have to have missed a subscription in " 
                    << "the remotely running RunTime, therefore the local and remote RunTime probably are out of sync.");*/
                }
            }
        }

        void RemoteObserver::onChannelUnpublished(const std::string& channelID)
        {
            Logger::printfln("RemoteObserver: onChannelUnpublished %s", channelID.c_str());
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
                // Can happen, if we miss a message for example (e.g., temporary loss of network).
                this->postError<ErrorRemoteRuntimeOutOfSync>();

                // Error, more unpublish channel more often than publish.
                // This should not happen. That would mean in the remote RunTime, channel was unpublished more than
                // published. Possibily the local and remote RunTime are out of sync.

                // ERROR! No publisher left, we unpublished more than we have published.
                // Local and remote RunTime out of sync? Severe error.
                // CLAID_THROW(Exception, "Error in RemoteObserver: Received message that indicates we shall unpublish channel with ID \"" << channelID << "\", "
                // << "however we have no publisher left for that channel. Therefore, we unpublished more than we published. Somehow, we have to have missed a publish in " 
                // << "the remotely running RunTime, therefore the local and remote RunTime probably are out of sync.");
            }
        }


        void RemoteObserver::onChannelDataReceivedFromRemoteRunTime(const std::string& targetChannel, TaggedData<BinaryData>& data)
        {
            Logger::printfln("RemoteObserver: onChannelDataReceivedFromRemoteRunTime %s", targetChannel.c_str());
            // Check if we have publisher for channel (we SHOULD! otherwise RunTimes out of sync,
            // because how could the remote RunTime have posted that data we just received in the first place?).
            
            auto it = this->publishedChannels.find(targetChannel);

            if(it == this->publishedChannels.end())
            {
                // Can happen, if we miss a message for example (e.g., temporary loss of network).
                this->postError<ErrorRemoteRuntimeOutOfSync>();
                return;
                // CLAID_THROW(Exception, "Error, received data from remote RunTime for channel with channelID \"" << targetChannel << "\", "
                // << "but we do not have a publisher for that channel. That should not be possible, because how could the remote RunTime have posted data to the channel "
                // << "in the first place, if no publisher is available ? ");
            }

            this->setIsDataReceivedFromRemoteRunTime(data);
            it->second.postBinaryData(data);
        }

        // Called, if a local Module (of this process) posted data to a channel, that has a subscriber in a remote run time.
        void RemoteObserver::onNewLocalDataInChannelThatRemoteRunTimeHasSubscribedTo(std::string channelID, ChannelData<Untyped> data)
        {
            	
            Logger::printfln("RemoteObserver %u: onNewLocalDataInChannelThatRemoteRunTimeHasSubscribedTo %s %d", this, channelID.c_str(), this->getUniqueIdentifier());
            // When there is a channel, that was subscribed to in the local RunTime AND the remote RunTime,
            // then it can happen that data get's send back and forth and a loop happens.
            // E.g.: 
            // 1st The local RunTime posts data to the channel.
            // 2nd Since there is a Module in the remote RunTime, that subscribed to that Channel, the data is send to the remote RunTime.
            // 3rd In the remote RunTime, the data is posted to the Channel in onChannelDataReceivedFromRemoteRunTime function of the RemoteObserver.
            // 4th This means, data is posted into the local channel in the remote RunTime. As the local RunTime (us) has subscribed to that Channel aswell,
            // onNewLocalDataInChannelThatRemoteRunTimeHasSubscribedTo is called in RemoteObserver of remote RunTime.
            // 5th The data is sent back to us again and posted to our local channel.
            // --> Go back to 1
            // // To solve this, we need to make sure that we do not react in this function to data that was posted in onChannelDataReceivedFromRemoteRunTime.
            TaggedData<BinaryData> taggedData = data.getBinaryData();
          
            if(this->isDataReceivedFromRemoteRunTime(taggedData))
            {
                Logger::printfln("Is data we received from remote run time, hence skipping");
                // TODO: HOW CAN I FIX THIS
                return;
            }
            const BinaryData& constBinaryData = taggedData.value();

            // Need to send to the remote RunTime
            // Get tagged binary data
            MessageDataBinary messageDataBinary;

            Message message = Message::CreateMessage<MessageHeaderChannelData, MessageDataBinary>();
             
            message.header->as<MessageHeaderChannelData>()->targetChannel = channelID;
            message.header->as<MessageHeaderChannelData>()->header = data.getHeader();
            Logger::printfln("Setting header timestamp %s %d", channelID.c_str(), data.getHeader().timestamp.toUnixTimestamp());
            // Set will serialize TaggedData<BinaryData>.
            // TaggedData holds the header (timestamp, sequenceID) and the binary data.
            // Thus, timestamp and sequenceID will be serialized, the binary data will be copied into a bigger
            // binary data buffer that contains timestamp, sequenceID and the binary data itself.    
            message.data->as<MessageDataBinary>()->setBinaryData(constBinaryData);

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
            //     CLAID_THROW(Exception, "Error in RemoteObserver: Received message that indicates we shall unpublish channel with ID \"" << channelID << "\", "
            //     << "however we have no publisher left for that channel. Therefore, we unpublished more than we published. Somehow, we have to have missed a publish in " 
            //     << "the remotely running RunTime, therefore the local and remote RunTime probably are out of sync.");
            // }
        }

        void RemoteObserver::sendMessage(Message message)
        {
            this->sendMessageChannel.post(message);
        }

        void RemoteObserver::initialize()
        {
            Logger::printfln("RemoteObserver initialize");
            this->registerPeriodicFunction("KeepAliveTimer", &RemoteObserver::sendPeriodicKeepAliveMessage, this, KEEP_ALIVE_INTERVAL_MILLISECONDS);
        }

        void RemoteObserver::terminate()
        {
            Logger::printfln("RemoteObserver %u terminate", this);
            for(auto it : this->subscribedChannelsWithCallback)
            {
                it.second.unsubscribe();
            }
            this->subscribedChannelsWithCallback.clear();

            for(auto it : this->subscribedChannels)
            {
                it.second.unsubscribe();
            }
            this->subscribedChannels.clear();

            for(auto it : this->publishedChannels)
            {
                it.second.unpublish();
            }
            this->publishedChannels.clear();                
        }

        void RemoteObserver::setIsDataReceivedFromRemoteRunTime(TaggedData<BinaryData>& data)
        {
            // Check if the data was tagged by us.
            std::string tag = RemoteObserver::IS_DATA_RECEIVED_FROM_REMOTE_TAG + std::to_string(this->getUniqueIdentifier()); 
            data.addTag(tag);
        }
            

        bool RemoteObserver::isDataReceivedFromRemoteRunTime(TaggedData<BinaryData> data)
        {
            std::string tag = RemoteObserver::IS_DATA_RECEIVED_FROM_REMOTE_TAG + std::to_string(this->getUniqueIdentifier()); 
            return data.hasTag(tag);
        }

        void RemoteObserver::sendPeriodicKeepAliveMessage()
        {
      
            if(this->hasKeepAliveMessageBeenSend)
            {
                if(!this->keepAliveResponseReceived)
                {
                    // Did not receive a reply to our previous message.
                    // Consider the connection to be lost.

                    this->postError<ErrorConnectionTimeout>();
                    return;
                }
            }

            Logger::printfln("RemoteObserver: Sending keep alive message");
            Message message = Message::CreateMessage<MessageHeaderKeepAlive, MessageDataEmpty>();
            this->sendMessage(message);
            this->keepAliveResponseReceived = false;
            this->hasKeepAliveMessageBeenSend = true;
        }

        void RemoteObserver::onKeepAliveMessage(const MessageHeaderKeepAlive& header, const MessageDataEmpty& data)
        {
            Logger::printfln("Received keep alive message. Sending response.");
            Message message = Message::CreateMessage<MessageHeaderKeepAliveResponse, MessageDataEmpty>();
            this->sendMessage(message);
        }

        void RemoteObserver::onKeepAliveResponse(const MessageHeaderKeepAliveResponse& header, const MessageDataEmpty& data)
        {
            Logger::printfln("RemoteObserver: Received keep alive message response. Connection is alive.");
            this->keepAliveResponseReceived = true;
        }
    }
}


       

