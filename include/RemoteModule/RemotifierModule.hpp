#pragma once

#include "RunTime/RunTime.hpp"

namespace portaible
{
    // A RemotifierModule is a Module that "remotifies" a Module running
    // in the current/local RunTime (this process) for another RunTime running remotely.
    // I.e.: It observes a locally running module and mirrors every action to the remote RunTime.
    // It is the counter part of the RemoteModule: Each RemoteModule running in the current/local RunTime
    // has a RemotifierModule in the remote RunTime.
    class RemotifierModule : public Module
    {
        private:

            // The module we observe.
            // Whenever this module does anything (e.g. subscribe or publish a channel),
            // the RemoteModule will forward this to the remote connected runtime accordingly.
            Module* localModule;

        public:

            void initialize()
            {
                observe(localModule);
                runLater(&RemotifierModule::remotify, this);
            }

            void remotify()
            {
                observe(localModule);
            }

    };
}