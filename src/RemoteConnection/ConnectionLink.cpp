#include "RemoteConnection/ConnectionLink.hpp"

namespace portaible
{
namespace RemoteConnection
{
    void ConnectionLink::link(ConnectionModule* connection, RemoteModule* module)
    {
        if(connection == nullptr)
        {
            PORTAIBLE_THROW(Exception, "Error, tried to link ConnectionModule and RemoteModule, but ConnectionModule is null.");
        }

        if(module == nullptr)
        {
            PORTAIBLE_THROW(Exception, "Error, tried to link ConnectionModule and RemoteModule, but RemoteModule is null.");
        }

        if(!connection->isInitialized())
        {
            PORTAIBLE_THROW(Exception, "Error, tried to link ConnectionModule of type " << connection->getModuleName() << " to RemoteModule"
            << " without ConnectionModule being initialized. Please call startModule before linkage");
        }

        if(!module->isInitialized())
        {
            PORTAIBLE_THROW(Exception, "Error, tried to link RemoteModule to ConnectionModule of type " << connection->getModuleName() 
            << " without RemoteModule being initialized. Please call startModule before linkage");
        }

        if(connection == nullptr)
        {
            PORTAIBLE_THROW(Exception, "Error: Tried to call ConnectionModule.link with a ConnectionModule that is null " << __FILE__ << __LINE__);
        }

        if(module == nullptr)
        {
            PORTAIBLE_THROW(Exception, "Error: Tried to call ConnectionModule.link with a RemoteModule that is null " << __FILE__ << __LINE__);
        }

        this->connection = connection;
        this->remoteModule = module;

        ChannelSubscriber<Message> subscriber = module->getSubscriberForReceptionOfMessages();


        this->remoteModule->setReceiveMessageChannel(connection->subscribeToReceiveChannel(subscriber));
        this->remoteModule->setSendMessageChannel(connection->registerToSendChannel());
    }

    void ConnectionLink::unlink()
    {
        if(this->connection == nullptr)
        {
            PORTAIBLE_THROW(Exception, "Error: Tried to call ConnectionModule.link with a ConnectionModule that is null " << __FILE__ << __LINE__);
        }

        if(this->remoteModule == nullptr)
        {
            PORTAIBLE_THROW(Exception, "Error: Tried to call ConnectionModule.link with a RemoteModule that is null " << __FILE__ << __LINE__);
        }

        this->remoteModule->unsubscribeReceiveMessageChannel();
        this->remoteModule->unpublishSendMessageChannel();
    }
}
}