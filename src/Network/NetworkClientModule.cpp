#include "Network/NetworkClientModule.hpp"


namespace claid
{
    namespace Network
    {
        void NetworkClientModule::getIPAndPortFromAddress(const std::string& address, std::string& ip, int& port)
        {
            size_t characterIndex = address.find(":");

            if(characterIndex == std::string::npos)
            {
                CLAID_THROW(Exception, "Error! Cannot get ip and port from address " << address << ". Address string is invalid.");
            }

            ip = address.substr(0, characterIndex);
            port = std::atoi(address.substr(characterIndex + 1).c_str());

            if(port < 0)
            {
                CLAID_THROW(Exception, "Error! Port " << port << " is invalid.");
            }

        }

    

        void NetworkClientModule::initialize()
        {
            NetworkModule::initialize();

            if(!connectToServer())
            {
                this->callError<ErrorConnectToAdressFailed>();
                return;
            }

        }

        bool NetworkClientModule::connectToServer()
        {
            std::string ip;
            int port;

            SocketClient socketClient;
            Logger::printfln("Trying to connect");

            getIPAndPortFromAddress(address, ip, port);
            Logger::printfln("Trying to connect to %s:%d", ip.c_str(), port);

            if(socketClient.connectTo(ip, port, this->timeoutInMs))
            {
                this->onConnectedSuccessfully(socketClient);
                return true;
            }
            
            // Failed to connect, close socket.
            Logger::printfln("Closing socket");
            socketClient.close();
            
            return false;
        }

        void NetworkClientModule::onConnectedSuccessfully(SocketClient socketClient)
        {
            Logger::printfln("Connected successfully");

            Logger::printfln("Creating RemoteConnected Entity.");

            this->remoteConnectedEntity = RemoteConnection::RemoteConnectedEntity::Create<SocketConnectionModule>(socketClient);
            Logger::printfln("RemoteConnectedEntity calling setup.");

            // First setup, then subscribe to errroChannel. Subscribing/publishing is only allowed during or after initialization
            // of the corresponding module.
            this->remoteConnectedEntity->setup();

            // Why do we store the ptr to remoteConnectedEntity here?
            // Because onClientLostConnection could be called multiple times (e.g., if read and write fail at the same time).
            // The remoteConnectedEntity will be deleted the first time that onClientConnectionLost is called and will be set to nullptr.
            // The second time it will be called, we could just check if this->remoteConnectedEntity is nullptr and return without doying anything.
            // However, if we reconnected faster than onClientLostConnection is called for the second time, remoteConnectedEntity will not be nullptr anymore
            // and we would canceled the reestablished connection. Hence, if remoteConnectedEntity is not nullptr, we check if it is the current entity.
            // If yes, that means that the reestablished connection has already lost connection again. If not, it means onConnectionLost was called twice for the old connection
            // and we can ignore it aswell.
            std::function<void (ChannelData<RemoteConnection::Error>)> function = std::bind(&NetworkClientModule::onErrorReceived, this, this->remoteConnectedEntity, std::placeholders::_1);
            this->errorChannel = remoteConnectedEntity->subscribeToErrorChannel(this->makeSubscriber(function));
            this->remoteConnectedEntity->start();
        }

        void NetworkClientModule::onErrorReceived(RemoteConnection::RemoteConnectedEntity* remoteConnectedEntity, ChannelData<RemoteConnection::Error> error)
        {
            this->onError(remoteConnectedEntity, error->value());
        }

        
        void NetworkClientModule::onError(RemoteConnection::RemoteConnectedEntity* remoteConnectedEntity, RemoteConnection::Error error)
        {
            if(error.is<ErrorConnectToAdressFailed>())
            {
                Logger::printfln("Error connecting to adress failed.");
                if(this->tryToReconnectAfterMs > 0)
                {
                    this->registerPeriodicFunction("PeriodicTryToReconnect", &NetworkClientModule::tryToReconnect, this, this->tryToReconnectAfterMs);
                }
            }
            else if(error.is<ErrorReadFromSocketFailed>())
            {
                Logger::printfln("Error read from socket failed.");
                // Read from socket failed. Connection lost.
                this->onConnectionLost(remoteConnectedEntity);
            }
            else if(error.is<RemoteConnection::ErrorRemoteRuntimeOutOfSync>())
            {
                Logger::printfln("NetworkClient: Error remote runtime out of sync.");
                this->onConnectionLost(remoteConnectedEntity);
            
            }
            else if(error.is<RemoteConnection::ErrorConnectionTimeout>())
            {
                Logger::printfln("NetworkClient: Error connection timeout.");
                this->onConnectionLost(remoteConnectedEntity);
            }
        }


        void NetworkClientModule::onConnectionLost(RemoteConnection::RemoteConnectedEntity* remoteConnectedEntity)
        {
            if(this->remoteConnectedEntity == nullptr)
            {
                // onConnectionLost might be called multiple times (e.g., if read and write
                // fail at the same time, two errors will be inserted into the channel, therefore onConnectionLost would be called twice.)
                return;
            }

            if(this->remoteConnectedEntity != remoteConnectedEntity)
            {
                // Connection was lost and onConnectionLost was called twice for the old entity.
                // However, connection was reestablished after onConnectionLost was called for the first time but before
                // it was called for the second time.
                // If we would close the connection now, that would mean we terminate the connection that was just reestablished.
                // See comment in onClientConnectedSuccessfully() (where we create the subscriber function).
                return;
            }

            Logger::printfln("NetworkClient: Client has lost connection. Shutting down.");
            this->remoteConnectedEntity->stop();
            this->remoteConnectedEntity->disintegrate();

            Logger::printfln("NetworkClient: deleting entity %u", this->remoteConnectedEntity);
            delete this->remoteConnectedEntity;
            this->remoteConnectedEntity = nullptr;

            if(this->tryToReconnectAfterMs > 0)
            {
                this->registerPeriodicFunction("PeriodicTryToReconnect", &NetworkClientModule::tryToReconnect, this, this->tryToReconnectAfterMs);
            }
        }

        void NetworkClientModule::tryToReconnect()
        {
            if(!this->isPeriodicFunctionRegistered("PeriodicTryToReconnect"))
            {
                // If tryToReconnectAfterMs is too small, 
                // it can happen that the next execution of the 
                // function is already scheduled, although a connection
                // has been successfully established the last time this funciton was called.
                // Therefore, if we do not check this here, it can happen that
                // connectToServer() is called multiple times successfully, which means
                // that we would connect to the same server twice (or more).
                // This actually happened sometimes in the tests, leading both 
                // the server and client to crash.

                // Therefore, if tryToReconnect() is called, but the periodicFunction (timer)
                // is not registered (anymore), we assume that we successfully established connection
                // in a prior call of this function.
                return;
            }
            Logger::printfln("Trying to reconnect");
            if(!connectToServer())
            {
                // Function will be called again after certain period (as it was registered as periodic function).
                return;
            }

            // On success, we unregister the function so that it is not called again.                 
            this->unregisterPeriodicFunction("PeriodicTryToReconnect");
        }


        void NetworkClientModule::onNetworkStateChangeRequested(const NetworkStateChangeRequest& networkStateChangeRequest)
        {
            if(networkStateChangeRequest.networkRequest == NetworkRequest::ENABLE_NETWORK)
            {
                if(this->disabled)
                {
                    return;
                }
                this->disable();
            }
            else
            {
                if(!this->disabled)
                {
                    return;
                }
                this->enable();
            }

        }
    }
}

REGISTER_MODULE(claid::Network::NetworkClientModule)