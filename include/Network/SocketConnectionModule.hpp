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
                SocketClient* socketClient = nullptr;
                SocketReaderModule* readerModule = nullptr;

                bool started = false;

                // Message internally only holds references to header and data,
                // thus passing it by copy is fine. We do it because the message
                // received in onSendMessage is const.
                void sendMessage(RemoteConnection::Message message);

            public:
                void start(SocketClient* socketClient);

        };
    }
}