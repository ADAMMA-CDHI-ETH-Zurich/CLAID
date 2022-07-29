#pragma once

#include "RunTime/RunTime.hpp"
#include "RemoteConnection/Message/Message.hpp"
#include "RemoteConnection/Error/Error.hpp"

namespace portaible
{
    namespace RemoteConnection
    {
        // A ConnectionModule is a module that can send and receive 
        // messages via a remote connection (e.g. socket, pipe, etc.).
        // It's an abstract class that can be derived from in order to implement
        // specific connections (e.g. SocketConnectionModule).
        // Derived classes need to implement the sendMessage function and they need to publish
        // the receiveChannel and post data to it whenever a new message was received (e.g. read from Socket).
        class ConnectionModule : public SubModule
        {
            private:
                Channel<Message> sendChannel;
                Channel<Message> receiveChannel;
                Channel<Error> errorChannel;

                static const std::string RECEIVE_CHANNEL;
                static const std::string SEND_CHANNEL;
                static const std::string ERROR_CHANNEL;

                // When someone wants to send a message via the remote connection.
                void onSendMessage(ChannelData<Message> message);

            protected:

                // Derived module needs to implement how to send the 
                // message via a remote connection.
                // Message internally only holds references to header and data,
                // thus passing it by copy is fine. We do it because the message
                // received in onSendMessage is const.
                // That wouldnt be a problem, but we need to serialize it to Binary.
                // As the reflect function is not const, we need to pass it by copy.
                virtual void sendMessage(Message message) = 0;

                virtual void setup();

                template<typename T>
                void postError()
                {
                    Error error;
                    error.set<T>();
                    this->errorChannel.post(error);
                }

                void initialize();

                // Should be exposed to the outside. The ConnectionModule is responsible for receiving the messages.
                Channel<Message> registerToReceiveChannel();


            public:
                ConnectionModule();


                Channel<Message> subscribeToReceiveChannel(ChannelSubscriber<Message> channelSubscriber);
                Channel<Message> registerToSendChannel();
                Channel<Error> subscribeToErrorChannel(ChannelSubscriber<Error> channelSubscriber);
                Channel<Error> registerToErrorChannel();


        };
    }
}
