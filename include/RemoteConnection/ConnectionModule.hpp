#pragma once

#include "RunTime/RunTime.hpp"
#include "RemoteConnection/Message/Message.hpp"

namespace portaible
{
    namespace RemoteConnection
    {
        // A ConnectionModule is a module that can send and receive 
        // that via a remote connection (e.g. socket, pipe, etc.).
        class ConnectionModule : public SubModule
        {
            private:
                LocalTypedChannel<Message> sendChannel;
                LocalTypedChannel<Message> receiveChannel;

            protected:

                // Derived module needs to implement how to send the 
                // message via a remote connection.
                virtual void sendMessage(const Message& message) = 0;
                virtual void start() = 0;

                // When message received from the remote connection.
                void onMessageReceived(Message& message);

                // When someone wants to send a message via the remote connection.
                void onSendMessage(ChannelData<Message> message);

               


            public:
                ConnectionModule();

                void initialize();

                Channel<Message> subscribeToReceiveChannel(ChannelSubscriber<Message> channelSubscriber);
                Channel<Message> registerToSendChannel();


        };
    }
}
