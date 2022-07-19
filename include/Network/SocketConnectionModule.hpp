#pragma once 

#include "RemoteConnection/RemoteConnectedClient.hpp"
#include "Network/Socket/SocketClient.hpp"
namespace portaible
{
    namespace Network
    {
        class SocketConnectionModule : public RemoteConnection::ConnectionModule
        {
            private:
                SocketClient client;

                void start();
                void sendMessage(const RemoteConnection::Message& message);

        };
    }
}