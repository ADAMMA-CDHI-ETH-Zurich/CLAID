#pragma once

#include "Network/Socket/Client/SocketClient.hpp"
#include "Network/Socket/Server/SocketServer.hpp"
#include "Network/Socket/Server/ClientAcceptModule.hpp"


#include "RemoteConnection/ConnectionLink.hpp"
#include "RemoteConnection/RemoteModule/RemoteModule.hpp"
#include "Network/SocketConnectionModule.hpp"
#include "Network/NetworkModule.hpp"

namespace portaible
{
    namespace Network
    {
        class NetworkServerModule : public NetworkModule
        {
            private:
                std::vector<RemoteConnection::RemoteModule*> remoteModules;
                std::vector<SocketConnectionModule*> socketConnections;
                std::vector<SocketClient*> clients;
                std::vector<RemoteConnection::ConnectionLink> links;

                void insert(RemoteConnection::RemoteModule*& remoteModule, SocketConnectionModule*& socketConnectionModule, SocketClient*& client, RemoteConnection::ConnectionLink& link)
                {
                    this->remoteModules.push_back(remoteModule);
                    this->socketConnections.push_back(socketConnectionModule);
                    this->clients.push_back(client);
                    this->links.push_back(link);
                }

                void onClientAccepted(ChannelData<SocketClient> socketClient)
                {
                    // Insert new
                    RemoteConnection::RemoteModule* remoteModule = new RemoteConnection::RemoteModule();
                    SocketConnectionModule* socketConnectionModule = new SocketConnectionModule();
                    SocketClient* newClient = new SocketClient(socketClient->value());
                    RemoteConnection::ConnectionLink link;



                    std::function<void (ChannelData<Error>)> callbackFunction = std::bind(&NetworkServerModule::onError, this, newClient, std::placeholders::_1);
                    socketConnectionModule->subscribeToErrorChannel();
                    
                    
                    link.link(socketConnectionModule, remoteModule);

                    remoteModule->startModule();
                    socketConnectionModule->startModule();

                    this->insert(remoteModule, socketConnectionModule, newClient, link);
                }

                void onError(SocketClient* client, ChannelData<Error> error)
                {

                }

            public:


        };
    }
}