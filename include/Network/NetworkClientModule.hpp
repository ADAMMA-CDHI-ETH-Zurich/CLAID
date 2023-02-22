#pragma once

#include "RemoteConnection/ConnectionLink.hpp"
#include "RemoteConnection/RemoteModule/RemoteModule.hpp"
#include "RemoteConnection/RemoteConnectedEntity.hpp"
#include "RemoteConnection/Error/ErrorRemoteRuntimeOutOfSync.hpp"
#include "RemoteConnection/Error/ErrorConnectionTimeout.hpp"
#include "Network/SocketConnectionModule.hpp"
#include "Network/NetworkModule.hpp"

namespace claid
{
    namespace Network
    {
        class NetworkClientModule : public NetworkModule
        {

            public:
                NetworkClientModule()
                {
                }

                NetworkClientModule(std::string ip, int port)
                {
                    this->address = ip + std::string(":") + std::to_string(port);
                }


            template<typename Reflector>
            void reflect(Reflector& r)
            {
                r.member("ConnectTo", this->address, "");
                r.member("TimeoutMsWhenTryingToConnect", this->timeoutInMs, "How long to wait for a response when (trying to) connect to a server.", static_cast<size_t>(3));
                r.member("TryToReconnectAfterMs", this->tryToReconnectAfterMs, "If the connection is lost (or could not be established), after how many ms should we try to reconnect? Set to 0 if reconnect should not happen.", static_cast<size_t>(1500));
            }

            private:
                std::string address;
                size_t timeoutInMs = 3;
                size_t tryToReconnectAfterMs = 200;

                bool connected = false;

                RemoteConnection::RemoteConnectedEntity* remoteConnectedEntity = nullptr;

                Channel<RemoteConnection::Error> errorChannel;
                
                void getIPAndPortFromAddress(const std::string& address, std::string& ip, int& port)
                {
                    size_t characterIndex = address.find(":");

                    if(characterIndex == std::string::npos)
                    {
                        CLAID_THROW(Exception, "Error! Cannot get ip and port from address " << address << ". Address string is invalid.");
                    }

                    ip = address.substr(0, characterIndex);
                    Logger::printfln("atoi %s", address.substr(characterIndex).c_str());
                    port = std::atoi(address.substr(characterIndex + 1).c_str());
                    Logger::printfln("atoi exec");

                    if(port < 0)
                    {
                        CLAID_THROW(Exception, "Error! Port " << port << " is invalid.");
                    }
                    Logger::printfln("atoi done");

                }

               

                void initialize()
                {
                    

                    if(!connectToServer())
                    {
                        this->callError<ErrorConnectToAdressFailed>();
                        return;
                    }
            


                }

                bool connectToServer()
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

                void onConnectedSuccessfully(SocketClient socketClient)
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

                void onErrorReceived(RemoteConnection::RemoteConnectedEntity* remoteConnectedEntity, ChannelData<RemoteConnection::Error> error)
                {
                    this->onError(remoteConnectedEntity, error->value());
                }

                template<typename T>
                void callError()
                {
                    RemoteConnection::Error error;
                    error.set<T>();
                    
                    // Make sure to call onError asynchronous. That way, we avoid recursive calls of onError.
                    // (E.g.: If error is ErrorConnectToAdressFailed, then we might try to reconnect. If that fails again,
                    // it would call onError again, and so on and so forth resulting in unbound recursion, which would eventually lead
                    // to a stack overflow).
                    this->callLater<NetworkClientModule, RemoteConnection::RemoteConnectedEntity*, RemoteConnection::Error>(&NetworkClientModule::onError, this, this->remoteConnectedEntity, error);
                }

                void onError(RemoteConnection::RemoteConnectedEntity* remoteConnectedEntity, RemoteConnection::Error error)
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


                void onConnectionLost(RemoteConnection::RemoteConnectedEntity* remoteConnectedEntity)
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

                void tryToReconnect()
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

                
        };

    }
}