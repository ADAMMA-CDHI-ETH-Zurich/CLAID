#include "Network/NetworkServerModule.hpp"

namespace claid
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
                std::bind(&NetworkServerModule::onErrorReceived, this, remoteConnectedEntity, std::placeholders::_1);
                    
            // First setup, then subscribe to errroChannel. Subscribing/publishing is only allowed during or after initialization
            // of the corresponding module.
            remoteConnectedEntity->setup();
            Channel<RemoteConnection::Error> errorChannel = remoteConnectedEntity->subscribeToErrorChannel(this->makeSubscriber(callbackFunction));
            remoteConnectedEntity->start();

            this->remoteConnectedEntities.push_back(remoteConnectedEntity);
            this->errorChannels.insert(std::make_pair(remoteConnectedEntity, errorChannel));
        }

        void NetworkServerModule::onErrorReceived(RemoteConnection::RemoteConnectedEntity* entity, ChannelData<RemoteConnection::Error> error)
        {
            this->onError(entity, error->value());
        }

        void NetworkServerModule::onError(RemoteConnection::RemoteConnectedEntity* entity, RemoteConnection::Error error)
        {
            if(error.is<ErrorReadFromSocketFailed>())
            {
                Logger::printfln("Error read from socket failed.");
                // Read from socket failed. Connection lost.
                this->onClientLostConnection(entity);
            }
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

        void NetworkServerModule::onClientLostConnection(RemoteConnection::RemoteConnectedEntity* entity)
        {
            Logger::printfln("A client lost connection, shutting it down now.");
            auto it = std::find(this->remoteConnectedEntities.begin(), this->remoteConnectedEntities.end(), entity);

            if(it == this->remoteConnectedEntities.end())
            {
                PORTAIBLE_THROW(Exception, "Error in NetworkServerModule: A client lost connection, so we wanted to shut it down safely. "
                "However, the client was not found in the list of known clients. This should not happen..");
            }

            this->remoteConnectedEntities.erase(it);


            auto it2 = this->errorChannels.find(entity);

            if(it2 == this->errorChannels.end())
            {
                PORTAIBLE_THROW(Exception, "Error in NetworkServerModule: A client lost connection, so we wanted to shut it down safely. "
                "However, the error channel for the client was not found in the list of known clients. This should not happen..");
                it2->second.unsubscribe();
            }

            this->errorChannels.erase(it2);


            entity->stop();
            entity->disintegrate();
            delete entity;
        }
    }
}

REGISTER_MODULE(claid::Network::NetworkServerModule)