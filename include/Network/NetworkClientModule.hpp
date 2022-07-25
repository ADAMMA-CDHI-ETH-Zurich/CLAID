#pragma once

#include "RemoteConnection/ConnectionLink.hpp"
#include "RemoteConnection/RemoteModule/RemoteModule.hpp"
#include "Network/SocketConnectionModule.hpp"
#include "Network/NetworkModule.hpp"

namespace portaible
{
    namespace Network
    {
        class NetworkClientModule : public NetworkModule
        {
            private:
                Network::SocketConnectionModule socketConnection;
                RemoteConnection::RemoteModule remoteModule;
                RemoteConnection::ConnectionLink link;
                SocketClient socketClient;    

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
                    socketConnection.startModule();
                    remoteModule.startModule();

                    socketConnection.waitForInitialization();
                    remoteModule.waitForInitialization();

                    // Subscribe to the error channel of the socket connection.
                    socketConnection.subscribeToErrorChannel(this->makeSubscriber(&NetworkClientModule::onErrorReceived, this));

                    link.link(&socketConnection, &remoteModule);

                    this->errorChannel = socketConnection.registerToErrorChannel();

                    std::string ip;
                    int port;

                    getIPAndPortFromAddress(address, ip, port);
                    if(!socketClient.connectTo(ip, port))
                    {
                        this->postError<ErrorConnectToAdressFailed>();
                        return;
                    }

                    socketConnection.start(&this->socketClient);
                }

                void onErrorReceived(ChannelData<RemoteConnection::Error> error)
                {
                    this->onError(error->value());
                }

                template<typename T>
                void postError()
                {
                    Error error;
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