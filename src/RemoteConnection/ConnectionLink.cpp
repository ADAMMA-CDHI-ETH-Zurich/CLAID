#include "RemoteConnection/ConnectionLink.hpp"

namespace claid
{
namespace RemoteConnection
{
    void ConnectionLink::link(ConnectionModule* connection, RemoteModule* module)
    {
        printf("ConnectionLink::link");
        if(connection == nullptr)
        {
            CLAID_THROW(Exception, "Error, tried to link ConnectionModule and RemoteModule, but ConnectionModule is null.");
        }

        if(module == nullptr)
        {
            CLAID_THROW(Exception, "Error, tried to link ConnectionModule and RemoteModule, but RemoteModule is null.");
        }

        if(!connection->isInitialized())
        {
            CLAID_THROW(Exception, "Error, tried to link ConnectionModule of type " << connection->getModuleName() << " to RemoteModule"
            << " without ConnectionModule being initialized. Please call startModule before linkage");
        }

        if(!module->isInitialized())
        {
            CLAID_THROW(Exception, "Error, tried to link RemoteModule to ConnectionModule of type " << connection->getModuleName() 
            << " without RemoteModule being initialized. Please call startModule before linkage");
        }

        if(connection == nullptr)
        {
            CLAID_THROW(Exception, "Error: Tried to call ConnectionModule.link with a ConnectionModule that is null " << __FILE__ << __LINE__);
        }

        if(module == nullptr)
        {
            CLAID_THROW(Exception, "Error: Tried to call ConnectionModule.link with a RemoteModule that is null " << __FILE__ << __LINE__);
        }

        this->connection = connection;
        this->remoteModule = module;

        ChannelSubscriber<Message> subscriber = module->getSubscriberForReceptionOfMessages();


        this->remoteModule->setReceiveMessageChannel(connection->subscribeToReceiveChannel(subscriber));
        this->remoteModule->setSendMessageChannel(connection->registerToSendChannel());
        this->remoteModule->setErrorChannel(connection->registerToErrorChannel());
    }

    void ConnectionLink::unlink()
    {
        if(this->connection == nullptr)
        {
            CLAID_THROW(Exception, "Error: Tried to call ConnectionModule.link with a ConnectionModule that is null " << __FILE__ << __LINE__);
        }

        if(this->remoteModule == nullptr)
        {
            CLAID_THROW(Exception, "Error: Tried to call ConnectionModule.link with a RemoteModule that is null " << __FILE__ << __LINE__);
        }

        
        this->remoteModule->unsubscribeReceiveMessageChannel();
        this->remoteModule->unpublishSendMessageChannel();
        this->remoteModule->unpublishErrorChannel();
    }
}
}