#pragma once

#include "RemoteConnection/ConnectionLink.hpp"
#include "RemoteConnection/RemoteModule/RemoteModule.hpp"
#include "RemoteConnection/RemoteConnectedEntity.hpp"
#include "Network/SocketConnectionModule.hpp"
#include "Network/NetworkModule.hpp"

namespace portaible
{
    namespace Network
    {
        class NetworkClientModule : public NetworkModule
        {
            PORTAIBLE_MODULE(NetworkClientModule)
            private:

                RemoteConnection::RemoteConnectedEntity* remoteConnectedEntity;

                std::string address;

                Channel<RemoteConnection::Error> errorChannel;
                
                void getIPAndPortFromAddress(const std::string& address, std::string& ip, int& port)
                {
                    size_t characterIndex = address.find(":");

                    if(characterIndex == std::string::npos)
                    {
                        PORTAIBLE_THROW(Exception, "Error! Cannot get ip and port from address " << address << ". Address string is invalid.");
                    }

                    ip = address.substr(0, characterIndex);
                    Logger::printfln("atoi %s", address.substr(characterIndex).c_str());
                    port = std::atoi(address.substr(characterIndex + 1).c_str());

                    if(port < 0)
                    {
                        PORTAIBLE_THROW(Exception, "Error! Port " << port << " is invalid.");
                    }
                }

                void initialize()
                {
                  

                    

                    std::string ip;
                    int port;

                    SocketClient socketClient;

                    getIPAndPortFromAddress(address, ip, port);
                    if(!socketClient.connectTo(ip, port))
                    {
                        this->callError<ErrorConnectToAdressFailed>();
                        return;
                    }
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
                    }
                    else if(error.is<ErrorReadFromSocketFailed>())
                    {

                    }
                }

            public:
                template<typename Reflector>
                void reflect(Reflector& r)
                {
                    r.member("ConnectTo", this->address, "");
                }
        };

    }
}