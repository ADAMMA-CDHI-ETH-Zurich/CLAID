#include "RemoteConnection/ConnectionLink.hpp"

namespace portaible
{
namespace RemoteConnection
{
    void ConnectionLink::link(ConnectionModule* connection, RemoteModule* module)
    {
        ChannelSubscriber<Message> subscriber = module->getSubscriberForMessageReceiveChannel();

        Channel<Message> subscribedChannel = connection->subscribeToReceiveChannel(subscriber);
        Channel<Message> publishedChannel = connection->registerToSendChannel();
    
        module->setReceiveMessageChannel(subscribedChannel);
        module->setSendMessageChannel(publishedChannel);
    }
}
}