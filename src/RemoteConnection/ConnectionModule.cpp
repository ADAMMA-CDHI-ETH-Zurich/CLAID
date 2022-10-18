
#include "RemoteConnection/ConnectionModule.hpp"

namespace claid
{
namespace RemoteConnection
{
    const std::string ConnectionModule::RECEIVE_CHANNEL = "ReceiveChannel";
    const std::string ConnectionModule::SEND_CHANNEL = "SendChannel";
    const std::string ConnectionModule::ERROR_CHANNEL = "ErrorChannel";

    ConnectionModule::ConnectionModule()
    {

    }
                
    // When someone wants to send a message via the remote connection.
    // Called by external module.
    void ConnectionModule::onSendMessage(ChannelData<Message> message)
    {
        Logger::printfln("onSendMessage");
        this->sendMessage(message->value());
    }

    void ConnectionModule::initialize()
    {
        Logger::printfln("ConnectionModule init");
        this->sendChannel = this->subscribeLocal<Message>(SEND_CHANNEL, &ConnectionModule::onSendMessage, this);
        this->receiveChannel = this->publishLocal<Message>(RECEIVE_CHANNEL);
        this->errorChannel = this->publishLocal<Error>(ERROR_CHANNEL);
        this->setup();
    }

    void ConnectionModule::terminate()
    {
        this->sendChannel.unsubscribe();
        this->receiveChannel.unpublish();
        this->errorChannel.unpublish();
    }

    Channel<Message> ConnectionModule::subscribeToReceiveChannel(ChannelSubscriber<Message> channelSubscriber)
    {
        return subscribeLocal<Message>(RECEIVE_CHANNEL, channelSubscriber);
    }

    Channel<Message> ConnectionModule::registerToReceiveChannel()
    {
        return publishLocal<Message>(RECEIVE_CHANNEL);
    }


    Channel<Message> ConnectionModule::registerToSendChannel()
    {
        return publishLocal<Message>(SEND_CHANNEL);
    }

    Channel<Error> ConnectionModule::subscribeToErrorChannel(ChannelSubscriber<Error> channelSubscriber)
    {
        return subscribeLocal<Error>(ERROR_CHANNEL, channelSubscriber);
    }

    Channel<Error> ConnectionModule::registerToErrorChannel()
    {
        return publishLocal<Error>(ERROR_CHANNEL);
    }

    void ConnectionModule::setup()
    {

    }



    
    
}
}
