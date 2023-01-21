#pragma once

#include "RemoteConnection/ConnectionLink.hpp"
#include "RemoteConnection/RemoteModule/RemoteModule.hpp"
#include "RemoteConnection/RemoteConnectedEntity.hpp"
#include "RemoteConnection/Error/ErrorRemoteRuntimeOutOfSync.hpp"
#include "Network/SocketConnectionModule.hpp"
#include "Network/NetworkModule.hpp"

namespace claid
{
    namespace Network
    {
        class NetworkClientModule : public NetworkModule
        {
            DECLARE_MODULE(NetworkClientModule)

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

                RemoteConnection::RemoteConnectedEntity* remoteConnectedEntity;

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
                    this->errorChannel = remoteConnectedEntity->subscribeToErrorChannel(this->makeSubscriber(&NetworkClientModule::onErrorReceived, this));
                    this->remoteConnectedEntity->start();
                }

                void onErrorReceived(ChannelData<RemoteConnection::Error> error)
                {
                    this->onError(error->value());
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
                    this->callLater<NetworkClientModule, RemoteConnection::Error>(&NetworkClientModule::onError, this, error);
                }

                void onError(RemoteConnection::Error error)
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
                        this->onConnectionLost();
                    }
                    if(error.is<RemoteConnection::ErrorRemoteRuntimeOutOfSync>())
                    {
                        Logger::printfln("NetworkClient: Error remote runtime out of sync.");
                        this->onConnectionLost();
                    
                    }
                }

                void onConnectionLost()
                {
                    Logger::printfln("Client has lost connection. Shutting down.");
                    this->remoteConnectedEntity->stop();
                    this->remoteConnectedEntity->disintegrate();
                    delete this->remoteConnectedEntity;

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