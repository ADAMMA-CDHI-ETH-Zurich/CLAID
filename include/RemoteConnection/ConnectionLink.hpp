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
                ConnectionModule* connection = nullptr;
                RemoteModule* remoteModule = nullptr;


            public:

                void link(ConnectionModule* connection, RemoteModule* module);
                void unlink();
        };
    }
}