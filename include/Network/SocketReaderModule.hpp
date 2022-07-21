#pragma once

#include "RunTime/RunTime.hpp"
#include "RemoteConnection/Message/Message.hpp"
#include "RemoteConnection/Error/Error.hpp"
#include "Network/Socket/SocketClient.hpp"
#include "Network/Error/NetworkErrors.hpp"
namespace portaible
{
    namespace Network
    {
        class SocketReaderModule : public SubModule
        {
            private:
                SocketClient* socketClient = nullptr;
                Channel<RemoteConnection::Message> messageReceivedChannel;
                Channel<RemoteConnection::Error> errorChannel;
                bool active;
                bool stopped;

                void run();

                template<typename T>
                void postError()
                {
                    Error error;
                    error.set<T>();
                    this->errorChannel.post(error);
                }

            public:
                SocketReaderModule();
                SocketReaderModule(SocketClient* client, Channel<RemoteConnection::Message> messageReceivedChannel, Channel<RemoteConnection::Error> errorChannel);

                void start();
                void stop();

                bool isStopped();

                
        };
    }
}