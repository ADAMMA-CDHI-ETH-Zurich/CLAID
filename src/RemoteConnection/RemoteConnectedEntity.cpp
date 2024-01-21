#include "RemoteConnection/RemoteConnectedEntity.hpp"

namespace claid
{
namespace RemoteConnection
{
    // This is a private constructor!
    RemoteConnectedEntity::RemoteConnectedEntity(ConnectionModule* connectionModule) : connectionModule(connectionModule)
    {
        Logger::logInfo("RemoteConnectedEntity constructor");
    }

    RemoteConnectedEntity::~RemoteConnectedEntity()
    {
        if(this->connectionModule != nullptr)
        {
            delete this->connectionModule;
        }
    }

    void RemoteConnectedEntity::setup()
    {
        printf("RemoteConnectedEntity::setup\n");
        this->connectionModule->startModule();
        this->connectionModule->waitForInitialization();
                printf("RemoteConnectedEntity::setup1\n");

        this->remoteModule.startModule();
        this->remoteModule.waitForInitialization();
                printf("RemoteConnectedEntity::setup2\n");
        this->link.link(this->connectionModule, &this->remoteModule);
    }

    void RemoteConnectedEntity::disintegrate()
    {
        this->remoteModule.stopModule();
        this->connectionModule->stopModule();

        // Only unlink afer stop.
        // Because if not stopped, remote module might still
        // use the channels to send data.
        this->link.unlink();
    }

    void RemoteConnectedEntity::start()
    {
        if(started)
        {
            CLAID_THROW(Exception, "Error, RemoteConnectedEntity::start() has been called while it is already running. Was start called multiple times?");
        }

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
        this->started = true;
    }

    void RemoteConnectedEntity::stop()
    {
        if(!started)
        {
            CLAID_THROW(Exception, "Error, RemoteConnectedEntity::stop() has been called while it is not running. Was start() called before?");
        }
        Logger::logInfo("Stopping remote module and connection module");
        this->remoteModule.stop();
        this->connectionModule->stop();
    }


    

    Channel<Error> RemoteConnectedEntity::subscribeToErrorChannel(ChannelSubscriber<Error> channelSubscriber)
    {
        return this->connectionModule->subscribeToErrorChannel(channelSubscriber);
    }

    Channel<Error> RemoteConnectedEntity::registerToErrorChannel()
    {
        return this->connectionModule->registerToErrorChannel();
    }

    RemoteConnectedEntityUniqueIdentifier RemoteConnectedEntity::getUniqueIdentifier()
    {
        return reinterpret_cast<intptr_t>(this);
    }
}
}