#pragma once

#include "RemoteConnection/RemoteModule/RemoteModule.hpp"
#include "RemoteConnection/ConnectionModule.hpp"
#include "RemoteConnection/ConnectionLink.hpp"

namespace portaible
{
    namespace RemoteConnection
    {
        // A RemoteConnectedEntity is "some entity" (whatsoever), that is connected to the current RunTime remotely.
        // A RemoteConnectedEntity can be communicated with using a ConnectionModule, and it's actions are mirrored
        // to the current RunTime by an RemoteModule. The RemoteModule and the ConnectionModule are connected using a ConnectionLink.

        class RemoteConnectedEntity
        {
            private:
                // Explicitly forbid copying.
                RemoteConnectedEntity(const RemoteConnectedEntity&) = delete;

                RemoteModule remoteModule;
                ConnectionModule* connectionModule;
                ConnectionLink link;


            public:
                RemoteConnectedEntity(ConnectionModule* connectionModule);

                void setup();
                void disintegrate();

        };
    }
}