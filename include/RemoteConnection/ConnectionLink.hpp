#pragma once
#include "RemoteConnection/ConnectionModule.hpp"
#include "RemoteConnection/RemoteModule/RemoteModule.hpp"
namespace portaible
{
    namespace RemoteConnection
    {
        class ConnectionLink 
        {
            private:
                ConnectionModule* remoteClient;
                RemoteModule* remoteModule;


            public:

                void link(ConnectionModule* client, RemoteModule* module);

        };
    }
}