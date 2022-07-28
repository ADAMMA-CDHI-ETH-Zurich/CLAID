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
            private:

                RemoteConnection::RemoteConnectedEntity* remoteConnectedEntitiy;

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
                    port = std::atoi(address.substr(characterIndex).c_str());

                    if(port < 0)
                    {
                        PORTAIBLE_THROW(Exception, "Error! Port " << port << " is invalid.");
                    }
                }

                void initialize()
                {
                  

                    // Subscribe to the error channel of the socket connection.
                    // socketConnection.subscribeToErrorChannel(this->makeSubscriber(&NetworkClientModule::onErrorReceived, this));
                    // this->errorChannel = socketConnection.registerToErrorChannel();

                    std::string ip;
                    int port;

                    SocketClient socketClient;

                    getIPAndPortFromAddress(address, ip, port);
                    if(!socketClient.connectTo(ip, port))
                    {
                        this->postError<ErrorConnectToAdressFailed>();
                        return;
                    }

                    
                    this->remoteConnectedEntitiy = RemoteConnection::RemoteConnectedEntity::Create<SocketConnectionModule>(socketClient);
                    this->remoteConnectedEntitiy->setup();
                }

                void onErrorReceived(ChannelData<RemoteConnection::Error> error)
                {
                    this->onError(error->value());
                }

                template<typename T>
                void postError()
                {
                    RemoteConnection::Error error;
                    error.set<T>();
                    this->errorChannel.post(error);
                }

                void onError(const RemoteConnection::Error& error)
                {
                    if(error.is<ErrorConnectToAdressFailed>())
                    {

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