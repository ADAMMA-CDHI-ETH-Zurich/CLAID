#include "Network/NetworkServerModule.hpp"

namespace portaible
{
    namespace Network
    {
        void NetworkServerModule::onClientAccepted(ChannelData<SocketClient> socketClient)
        {   
            Logger::printfln("onClientAccepted");
            RemoteConnection::RemoteConnectedEntity* remoteConnectedEntity = 
                RemoteConnection::RemoteConnectedEntity::Create<SocketConnectionModule>(socketClient->value());
            
    
            // We bind the remoteConnectedEntity to the function aswell. 
            // In that case, whenever onError is called, the NetworkServerModule also knows in which Entity the error occured.
            std::function<void (ChannelData<RemoteConnection::Error>)> callbackFunction =
                std::bind(&NetworkServerModule::onError, this, remoteConnectedEntity, std::placeholders::_1);
                    
            // First setup, then subscribe to errroChannel. Subscribing/publishing is only allowed during or after initialization
            // of the corresponding module.
            remoteConnectedEntity->setup();
            Channel<RemoteConnection::Error> errorChannel = remoteConnectedEntity->subscribeToErrorChannel(this->makeSubscriber(callbackFunction));
            remoteConnectedEntity->start();

            this->remoteConnectedEntities.push_back(remoteConnectedEntity);
            this->errorChannels.insert(std::make_pair(remoteConnectedEntity, errorChannel));
        }

        void NetworkServerModule::onError(RemoteConnection::RemoteConnectedEntity* entity, ChannelData<RemoteConnection::Error> error)
        {

        }

        void NetworkServerModule::onClientAcceptError(ChannelData<RemoteConnection::Error> error)
        {

        }

        void NetworkServerModule::initialize()
        {
            if(!server.bindTo(port))
            {
                Logger::printfln("SocketServer error: %s (%s)", this->server.getLastError().errorTypeAsString().c_str(), this->server.getLastError().errorString.c_str());
                PORTAIBLE_THROW(Exception, "Failed to start server on port " << this->port);
            }

            this->clientAcceptModule.startModule();
            this->clientAcceptModule.waitForInitialization();

            this->clientAcceptChannel = this->clientAcceptModule.subscribeToClientAcceptChannel(this->makeSubscriber(&NetworkServerModule::onClientAccepted, this));
            this->clientAcceptErrorChannel = this->clientAcceptModule.subscribeToErrorChannel(this->makeSubscriber(&NetworkServerModule::onClientAcceptError, this));

            

            this->clientAcceptModule.start(&this->server);
        }
    }
}

PORTAIBLE_SERIALIZATION(portaible::Network::NetworkServerModule)