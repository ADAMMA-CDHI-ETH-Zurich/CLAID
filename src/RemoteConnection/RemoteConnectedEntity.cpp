#include "RemoteConnection/RemoteConnectedEntity.hpp"

namespace portaible
{
namespace RemoteConnection
{
    // This is a private constructor!
    RemoteConnectedEntity::RemoteConnectedEntity(ConnectionModule* connectionModule) : connectionModule(connectionModule)
    {
        
    }

    

    void RemoteConnectedEntity::setup()
    {
        this->link.link(this->connectionModule, &this->remoteModule);

        this->connectionModule->startModule();
        this->remoteModule.startModule();

        this->connectionModule->waitForInitialization();
        this->remoteModule.waitForInitialization();
    }

    void RemoteConnectedEntity::disintegrate()
    {
        this->link.unlink();
        this->remoteModule.stopModule();
    }

    Channel<Error> RemoteConnectedEntity::subscribeToErrorChannel(ChannelSubscriber<Error> channelSubscriber)
    {
        return this->connectionModule->subscribeToErrorChannel(channelSubscriber);
    }

    Channel<Error> RemoteConnectedEntity::registerToErrorChannel()
    {
        return this->connectionModule->registerToErrorChannel();
    }
}
}