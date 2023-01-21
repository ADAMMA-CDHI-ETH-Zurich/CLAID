#include "Network/NetworkServerModule.hpp"
#include "RemoteConnection/Error/ErrorRemoteRuntimeOutOfSync.hpp"
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

            this->onClientConnectedChannel.post(remoteConnectedEntity->getUniqueIdentifier());
        }

        void NetworkServerModule::onErrorReceived(RemoteConnection::RemoteConnectedEntity* entity, ChannelData<RemoteConnection::Error> error)
        {
            this->onError(entity, error->value());
        }

        void NetworkServerModule::onError(RemoteConnection::RemoteConnectedEntity* entity, RemoteConnection::Error error)
        {
            Logger::printfln("NetworkServer moduel received error!!");

            if(error.is<ErrorReadFromSocketFailed>())
            {
                Logger::printfln("Error read from socket failed.");
                // Read from socket failed. Connection lost.
                this->onClientLostConnection(entity);
            }
            else if(error.is<claid::RemoteConnection::ErrorRemoteRuntimeOutOfSync>())
            {
                Logger::printfln("NetworkServer: Error remote runtime out of sync.");
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
                CLAID_THROW(Exception, "Failed to start server on port " << this->port);
            }

            this->clientAcceptModule.startModule();
            this->clientAcceptModule.waitForInitialization();

            this->clientAcceptChannel = this->clientAcceptModule.subscribeToClientAcceptChannel(this->makeSubscriber(&NetworkServerModule::onClientAccepted, this));
            this->clientAcceptErrorChannel = this->clientAcceptModule.subscribeToErrorChannel(this->makeSubscriber(&NetworkServerModule::onClientAcceptError, this));


            this->onClientConnectedChannel = this->publish<RemoteConnection::RemoteConnectedEntityUniqueIdentifier>("/CLAID/LOCAL/OnNetworkClientConnected");
            this->onClientDisconnectedChannel = this->publish<RemoteConnection::RemoteConnectedEntityUniqueIdentifier>("/CLAID/LOCAL/OnNetworkClientDisconnected");
             

            this->clientAcceptModule.start(&this->server);
        }

        void NetworkServerModule::onClientLostConnection(RemoteConnection::RemoteConnectedEntity* entity)
        {
           
            Logger::printfln("Client %ul lost connection, shutting it down now.", entity);
            
        
      
            onClientDisconnectedChannel.post(entity->getUniqueIdentifier());
            
            auto it = std::find(this->remoteConnectedEntities.begin(), this->remoteConnectedEntities.end(), entity);

            if(it == this->remoteConnectedEntities.end())
            {
                // It can happen that this get's called two times per entity.
                // Connection is considered to be lost, if read or write fail.
                // Of course, that can also happen concurrently and at the same time.
                // Hence, if we cannot find the entity anymore, assume it has lost connection
                // and was removed before already.
                return;
                //CLAID_THROW(Exception, "Error in NetworkServerModule: A client lost connection, so we wanted to shut it down safely. "
                //"However, the client was not found in the list of known clients. This should not happen..");
            }

            this->remoteConnectedEntities.erase(it);


            auto it2 = this->errorChannels.find(entity);

            if(it2 == this->errorChannels.end())
            {
                CLAID_THROW(Exception, "Error in NetworkServerModule: A client lost connection, so we wanted to shut it down safely. "
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