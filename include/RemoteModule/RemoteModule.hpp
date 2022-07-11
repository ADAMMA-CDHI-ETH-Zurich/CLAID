#pragma once

#include "RunTime/RunTime.hpp"

namespace portaible
{
    // A RemoteModule is a module that mirrors a Module running in another RunTime (remotely).
    // Whenever a channel is published/subscribed or data is posted by
    // the mirrored module, the RemoteModule will perform the same actions locally.
    class RemoteModule : public Module
    {
        private:
            //SocketClientModule socketClientModule

        void onData()
        {

        }
    };
}