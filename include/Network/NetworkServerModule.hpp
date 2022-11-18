#pragma once

#include "Network/Socket/Client/SocketClient.hpp"
#include "Network/Socket/Server/SocketServer.hpp"
#include "Network/Socket/Server/ClientAcceptModule.hpp"
#include "Network/SocketConnectionModule.hpp"
#include "Network/NetworkModule.hpp"

#include "RemoteConnection/ConnectionLink.hpp"
#include "RemoteConnection/RemoteModule/RemoteModule.hpp"
#include "RemoteConnection/RemoteConnectedEntity.hpp"


namespace claid
{
    namespace Network
    {
        class NetworkServerModule : public NetworkModule
        {
            DECLARE_MODULE(NetworkServerModule)
            
            public:

                NetworkServerModule()
                {

                }
                
                NetworkServerModule(int port) : port(port)
                {

                }

                template<typename Reflector>
                void reflect(Reflector& r)
                {
                    r.member("Port", this->port, "");
                }

            private:
                
                size_t port;
                // We will push clients that connected to this Server to Channel /CLAID/LOCAL/OnNetworkClientConnected
                // and clients that disconnected to /CLAID/LOCAL/OnNetworkClientDisconnected
                // This can be useful for monitoring tools, e.g. to see externally connected RunTimes.
                // Note, that /CLAID/LOCAL is a hidden namespace, therefore these channels will not be mirrored to any 
                // remotely connected instance of CLAID.
                Channel<RemoteConnection::RemoteConnectedEntityUniqueIdentifier> onClientConnectedChannel;
                Channel<RemoteConnection::RemoteConnectedEntityUniqueIdentifier> onClientDisconnectedChannel;

                std::vector<RemoteConnection::RemoteConnectedEntity*> remoteConnectedEntities;
                std::map<RemoteConnection::RemoteConnectedEntity*, Channel<RemoteConnection::Error>> errorChannels;

                Channel<RemoteConnection::Error> clientAcceptErrorChannel;
                Channel<SocketClient> clientAcceptChannel;

                ClientAcceptModule clientAcceptModule;
                SocketServer server;

                void onClientAccepted(ChannelData<SocketClient> socketClient);
                void onClientAcceptError(ChannelData<RemoteConnection::Error> error);
                void onErrorReceived(RemoteConnection::RemoteConnectedEntity* entity, ChannelData<RemoteConnection::Error> error);
                void onError(RemoteConnection::RemoteConnectedEntity* entity, RemoteConnection::Error error);
                void initialize();

                void onClientLostConnection(RemoteConnection::RemoteConnectedEntity* entity);

               


        };
    }
}