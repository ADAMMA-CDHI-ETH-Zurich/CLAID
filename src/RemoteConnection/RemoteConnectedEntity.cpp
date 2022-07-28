#include "RemoteConnection/RemoteConnectedEntity.hpp"

namespace portaible
{
namespace RemoteConnection
{
    // This is a private constructor!
    RemoteConnectedEntity::RemoteConnectedEntity(ConnectionModule* connectionModule) : connectionModule(connectionModule)
    {
        
    }

    // We use factory function to make sure that the RemoteConnectedEntity uses
    // a ConnectionModule that was created by itself. I.e.: We make sure we have ownership
    // over the ConnectionModule and it's not exposed to "the outside world".
    template<typename ConnectionModuleType, typename... arguments>
    static RemoteConnectedEntity* Create(arguments... args)
    {
        return new RemoteConnectedEntity(new ConnectionModuleType(args...));
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