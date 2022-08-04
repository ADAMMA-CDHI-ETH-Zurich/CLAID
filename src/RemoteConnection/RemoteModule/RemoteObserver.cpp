#include "RemoteConnection/RemoteModule/RemoteObserver.hpp"
#include "RemoteConnection/Message/MessageData/MessageDataBinary.hpp"

namespace portaible
{
    namespace RemoteConnection
    {
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
            const MessageHeader& messageHeader = *messageRef.header.get();
            const MessageData& messageData = *messageRef.data.get();


            if(messageHeader.is<MessageHeaderChannelUpdate>())
            {
                const MessageHeaderChannelUpdate& updateHeader = *messageHeader.as<MessageHeaderChannelUpdate>();
                // if(!messageRef.data.is<MessageDataString>())
                // {
                //     PORTAIBLE_THROW(Exception, "Error! RemoteObserver received message of type " << messageHeader.getClassName() << " and data of type " 
                //     << messageData.getClassName() << ", but data of type " << MessageDataString::staticGetGlassName());
                // }

                // this->onChannelUpdate(*);
            }
            else 
            {
                PORTAIBLE_THROW(Exception, "Error! RemoteObserver received unsupported message of type " << messageHeader.getClassName() << ".");
                /* code */
            }
            

            // Logger::printfln("RECEIVED MESSAGE\n");

            // std::vector<std::string> channelNames;
            // Message m = message->value();
            // m.data->as<MessageDataBinary>()->get<std::vector<std::string>>(channelNames);

            // for(std::string name : channelNames)
            // {
            //     Logger::printfln("Channel name: %s", name.c_str());
            // }
        }    

         void onChannelUpdate(MessageHeaderChannelUpdate::UpdateType updateType, const MessageDataString& data);
                void onChannelPublished(const std::string& channelID);
    }
}


       

