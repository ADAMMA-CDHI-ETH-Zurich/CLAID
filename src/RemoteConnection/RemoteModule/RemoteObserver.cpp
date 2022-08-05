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

            // Calls onChannelUpdate if message.header is MessageHeaderChannelUpdate and message.data is MessageDataString.
            // Throws exception, if header types match (message.header is MessageHeaderChannelUpdate), but data types do not (message.data is not MessageDataString).
            if(callFunctionIfSignatureMatchesThrowExceptionIfWrongData<MessageHeaderChannelUpdate, MessageDataString>(messageRef, &RemoteObserver::onChannelUpdate, this)) return;
           // if(callFunctionIfSignatureMatchesThrowExceptionIfWrongData<MessageHeaderChannelData, MessageDataBinary>(messageRef, &RemoteObserver::onChannelData, this)) return;


            PORTAIBLE_THROW(Exception, "Error! RemoteObserver received message with unexpected header \"" << messageRef.header->getClassName() << "\".");

        }    

         void RemoteObserver::onChannelUpdate(const MessageHeaderChannelUpdate&, const MessageDataString& data)
         {

         }
                
    }
}


       

