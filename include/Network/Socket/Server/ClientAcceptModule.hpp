#pragma once

#include "RunTime/RunTime.hpp"
#include "Network/Socket/Client/SocketClient.hpp"
#include "Network/Socket/Server/SocketServer.hpp"

#include "RemoteConnection/Error/Error.hpp"

namespace portaible
{
    namespace Network
    {
        class ClientAcceptModule : public SubModule
        {
            private:
                Channel<SocketClient> clientAcceptChannel;
                Channel<RemoteConnection::Error> errorChannel;

                static const std::string CLIENT_ACCEPT_CHANNEL;
                static const std::string ERROR_CHANNEL;
        
                bool active = false;

                void run();
                void initialize();

            public:
                SocketServer* server = nullptr;

                void subscribeToClientAcceptChannel(ChannelSubscriber<SocketClient> subscriber);
                void subscribeToErrorChannel(ChannelSubscriber<RemoteConnection::Error> subscriber);
                void start(SocketServer* server);
                
                
        };
    }
}