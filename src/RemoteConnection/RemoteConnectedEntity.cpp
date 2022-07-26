#include "RemoteConnection/RemoteConnectedEntity.hpp"

namespace portaible
{
namespace RemoteConnection
{
    RemoteConnectedEntity::RemoteConnectedEntity(ConnectionModule* connectionModule) : connectionModule(connectionModule)
    {
        
    }

    void RemoteConnectedEntity::setup()
    {
        this->link.link(this->connectionModule, &this->remoteModule);

        this->remoteModule.startModule();
        this->remoteModule.waitForInitialization();
    }

    void RemoteConnectedEntity::disintegrate()
    {
        this->link.unlink();
        this->remoteModule.stopModule();
    }
}
}