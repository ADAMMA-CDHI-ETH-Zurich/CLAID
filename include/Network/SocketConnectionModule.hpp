#pragma once 

#include "RemoteConnection/ConnectionModule.hpp"
#include "Network/Socket/Client/SocketClient.hpp"
#include "Network/SocketReaderModule.hpp"
namespace portaible
{
    namespace Network
    {
        class SocketConnectionModule : public RemoteConnection::ConnectionModule
        {
            private:
                SocketClient socketClient;
                SocketReaderModule readerModule;

                bool started = false;

                // Message internally only holds references to header and data,
                // thus passing it by copy is fine. We do it because the message
                // received in onSendMessage is const.
                void sendMessage(RemoteConnection::Message message);
                void setup();

            public:
                SocketConnectionModule(SocketClient socketClient);
                void start();

        };
    }
}