#pragma once

#include "RemoteConnection/RemoteModule/RemoteModule.hpp"
#include "RemoteConnection/ConnectionModule.hpp"
#include "RemoteConnection/ConnectionLink.hpp"
#include "RemoteConnection/Error/Error.hpp"

namespace portaible
{
    namespace RemoteConnection
    {
        // A RemoteConnectedEntity is "some entity" (whatsoever), that is connected to the current RunTime remotely.
        // A RemoteConnectedEntity can be communicated with using a ConnectionModule, and it's actions are mirrored
        // to the current RunTime by a RemoteModule. The RemoteModule and the ConnectionModule are connected using a ConnectionLink.

        class RemoteConnectedEntity
        {
            private:
                // Explicitly forbid copying.
                RemoteConnectedEntity(const RemoteConnectedEntity&) = delete;

                RemoteModule remoteModule;
                ConnectionModule* connectionModule;
                ConnectionLink link;

                RemoteConnectedEntity(ConnectionModule* connectionModule);

            public:
                RemoteConnectedEntity() = delete;

                template<typename ConnectionModuleType, typename... arguments>
                static RemoteConnectedEntity* Create(arguments...);

                void setup();
                void disintegrate();

                Channel<Error> subscribeToErrorChannel(ChannelSubscriber<Error> subscriber);

        };
    }
}