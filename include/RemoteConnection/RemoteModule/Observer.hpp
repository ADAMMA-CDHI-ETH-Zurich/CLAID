#pragma once

#include "RunTime/RunTime.hpp"
#include "RemoteConnection/Message/Message.hpp"

namespace claid
{
    namespace RemoteConnection
    {
        class Observer : public SubModule
        {
            protected:
                Channel<Message> sendMessageChannel;
                bool sendMessageChannelSet = false;

            public:
                
                void setSendMessageChannel(Channel<RemoteConnection::Message> sendMessageChannel) 
                {
                    this->sendMessageChannel = sendMessageChannel;
                    this->sendMessageChannelSet = true;
                }

                void sendMessage(Message& message)
                {
                    if(!this->sendMessageChannelSet)
                    {
                        CLAID_THROW(Exception, "Error! Observer tried to send a message, however a channel to send messages was never set."
                        "Please make sure to call setSendMessageChannel of RemoteModule " << __FILE__ << " " << __LINE__);
                    }
                    Logger::printfln("SendMessageChannel post");
                    this->sendMessageChannel.post(message);
                }


        };
    }
}