#include "RemoteConnection/RemoteConnectedEntity.hpp"

namespace portaible
{
namespace RemoteConnection
{
    // This is a private constructor!
    RemoteConnectedEntity::RemoteConnectedEntity(ConnectionModule* connectionModule) : connectionModule(connectionModule)
    {
        Logger::printfln("RemoteConnectedEntity constructor");
    }

    

    void RemoteConnectedEntity::setup()
    {
        Logger::printfln("Start connection module");
        this->connectionModule->startModule();
        this->connectionModule->waitForInitialization();
        
        Logger::printfln("Start connection module2 ");
        this->remoteModule.startModule();
        this->remoteModule.waitForInitialization();
        this->link.link(this->connectionModule, &this->remoteModule);
    }

    void RemoteConnectedEntity::start()
    {
        // Why start not in setup ? 
        // Because, maybe an external module wants to subscribe to the error channel (which is actually the
        // error channel of the ConnectionModule), before tha ConnectionModule starts doing it's job (that's also
        // the reason why the ConnectionModule has a setup() and a separate start() function).
        // It is only allowed to subscribe/publish channels during initialization/setup. Therefore,
        // an external Module (e.g. the NetworkServerModule), that uses/starts the RemoteConnectedEntity, 
        // can only subscribe to the error channel AFTER calling setup() (which calls startModule() of the ConnectionModule).
        // Thus, after calling startModule(), the external module can subscribe to the error channel and then
        this->connectionModule->start();
        this->remoteModule.start();
    }

    void RemoteConnectedEntity::disintegrate()
    {
        this->link.unlink();
        this->remoteModule.stopModule();
        // TODO: Implement connection module stop
        PORTAIBLE_THROW(Exception, "Disintegrate was called but connectionmodule.stop was not implemented yet.");
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