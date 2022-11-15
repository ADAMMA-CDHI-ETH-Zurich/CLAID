#include "Network/Socket/Server/ClientAcceptModule.hpp"

namespace claid
{
    namespace Network
    {
        const std::string ClientAcceptModule::CLIENT_ACCEPT_CHANNEL = "ClientAcceptChannel";
        const std::string ClientAcceptModule::ERROR_CHANNEL = "ErrorChannel";


        void ClientAcceptModule::initialize()
        {
            this->clientAcceptChannel = this->publishLocal<SocketClient>(CLIENT_ACCEPT_CHANNEL);
        }

        Channel<SocketClient> ClientAcceptModule::subscribeToClientAcceptChannel(ChannelSubscriber<SocketClient> subscriber)
        {
            return this->subscribeLocal<SocketClient>(CLIENT_ACCEPT_CHANNEL, subscriber);
        }
        
        Channel<RemoteConnection::Error> ClientAcceptModule::subscribeToErrorChannel(ChannelSubscriber<RemoteConnection::Error> subscriber)
        {
            return this->subscribeLocal<RemoteConnection::Error>(ERROR_CHANNEL, subscriber);
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
            Logger::printfln("ClientAcceptModule::run");
            while(this->active)
            {
                SocketClient client;

                if(!this->server->accept(client))
                {
                    // TODO: POST ERROR ACCEPT FAILED
                    CLAID_THROW(Exception, "Failed to accept client");
                }
                Logger::printfln("Post");
                this->clientAcceptChannel.post(client);
            }
        }
    }
}