#pragma once

#include "RemoteConnection/ConnectionLink.hpp"
#include "RemoteConnection/RemoteModule/RemoteModule.hpp"
#include "Network/SocketConnectionModule.hpp"

namespace portaible
{
    namespace Network
    {
        class NetworkModuleClient : public NetworkModule
        {
            private:
                Network::SocketConnectionModule socketConnection;
                RemoteConnection::RemoteModule remoteModule;
                
                RemoteConnection::ConnectionLink link;
                
        
                void initialize()
                {
                    link.link(&remoteClient, &remoteModule);
                    
                    // socketConnection.connectTo(ip, port);
                }
        };

    }
}