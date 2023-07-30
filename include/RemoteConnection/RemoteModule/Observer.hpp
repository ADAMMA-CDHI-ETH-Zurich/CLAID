#pragma once

#include "RunTime/RunTime.hpp"
#include "RemoteConnection/Message/Message.hpp"
#include "RemoteConnection/Error/Error.hpp"

namespace claid
{
    namespace RemoteConnection
    {
        class Observer : public SubModule
        {
            protected:
                Channel<Message> sendMessageChannel;
                Channel<Error> errorChannel;
                bool sendMessageChannelSet = false;
                bool errorChannelSet = false;

                template<typename T>
                void postError()
                {
                    if(!errorChannelSet)
                    {
                        CLAID_THROW(Exception, "Error in Observer, cannot post error since error channel was not set before.");
                    }

                    Error error;
                    error.set<T>();
                    this->errorChannel.post(error);
                }

            public:
                
                void setSendMessageChannel(Channel<RemoteConnection::Message> sendMessageChannel) 
                {
                    printf("SendMessageChannelSet %s\n", this->getModuleName().c_str());
                    this->sendMessageChannel = sendMessageChannel;
                    this->sendMessageChannelSet = true;
                }

                void setErrorChannel(Channel<Error> errorChannel)
                {
                    this->errorChannel = errorChannel;
                    this->errorChannelSet = true;
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