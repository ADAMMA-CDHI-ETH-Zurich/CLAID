#pragma once
#include "Network/NetworkModule.hpp"
#include "Network/SocketClient.hpp"

namespace portaible
{
    namespace Network
    {
        class NetworkModuleClient
        {
            private:
                SocketClient client;


            public:
                virtual void asyncSendMessage(Message message)
                {
                    // Client write message.. how ? 

                }

                virtual void initialize()
                {
                    
                }

        };
    }
}