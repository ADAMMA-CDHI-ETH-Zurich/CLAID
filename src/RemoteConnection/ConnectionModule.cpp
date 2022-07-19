#pragma once

#include "RemoteConnection/ConnectionModule.hpp"

namespace portaible
{
namespace RemoteConnection
{
    ConnectionModule::ConnectionModule()
    {

    }
                
    // When message received from the remote connection.
    void ConnectionModule::onMessageReceived(Message& message)
    {
        this->receiveChannel.post(message);
    }

    // When someone wants to send a message via the remote connection.
    void ConectionModule::onSendMessage(ChannelData<Message> message)
    {
        this->sendMessage(message->value());
    }
    

    

    void ConectionModule::initialize()
    {
        this->subscribe(this->sendChannel, &RemoteConnectedClient::onSendMessage, this);
        this->start();
    }

    Channel<Message> ConectionModule::subscribeToReceiveChannel(ChannelSubscriber<Message> channelSubscriber)
    {
        return this->receiveChannel.subscribe(channelSubscriber);
    }

    Channel<Message> ConectionModule::registerToSendChannel()
    {
        return this->sendChannel.publish();
    }


    
}
}
