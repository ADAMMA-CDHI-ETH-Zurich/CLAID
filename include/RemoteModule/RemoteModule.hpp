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
            onPublish();
            onSubscribe();
        }

        void onPublish(const std::string& channelID)
        {
            // Publish means that the Module running in the remote RunTime, that we mirror,
            // published a channel. Thus, we need to publish it aswell..
            // How do we do that?
            Channel<Untyped> = this->publish<Untyped>(channelID);
        }
    };
}