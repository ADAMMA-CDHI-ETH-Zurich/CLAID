#include "Network/Socket/Server/ClientAcceptModule.hpp"

namespace portaible
{
    namespace Network
    {
        const std::string ClientAcceptModule::CLIENT_ACCEPT_CHANNEL = "ClientAcceptChannel";


        void ClientAcceptModule::initialize()
        {
            this->clientAcceptChannel = this->publishLocal<SocketClient>(CLIENT_ACCEPT_CHANNEL);
        }

        void ClientAcceptModule::subscribeToClientAcceptChannel(ChannelSubscriber<SocketClient> subscriber)
        {
            this->subscribeLocal<SocketClient>(CLIENT_ACCEPT_CHANNEL);
        }
        
        void ClientAcceptModule::start(SocketServer* server)
        {
            this->server = server;
            this->active = true;
            // Start might be called by another thread. Make sure we switch to our own thread.
            this->callLater(&ClientAcceptModule::run, this);
        }


        void ClientAcceptModule::run()
        {
            while(this->active)
            {
                SocketClient client;

                if(!this->server->accept(client))
                {
                    // TODO: POST ERROR ACCEPT FAILED
                }
                this->clientAcceptChannel.post(client);
            }
        }
    }
}