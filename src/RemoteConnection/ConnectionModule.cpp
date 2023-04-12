
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
        Logger::printfln("send message done");
    }

    void ConnectionModule::initialize()
    {
        Logger::printfln("ConnectionModule init 1");
        this->sendChannel = this->subscribeLocal<Message>(SEND_CHANNEL, &ConnectionModule::onSendMessage, this);
        Logger::printfln("ConnectionModule init 2");
        this->receiveChannel = this->publishLocal<Message>(RECEIVE_CHANNEL);
        Logger::printfln("ConnectionModule init 3");
        this->errorChannel = this->publishLocal<Error>(ERROR_CHANNEL);
        Logger::printfln("ConnectionModule calling setup");
        this->setup();
        Logger::printfln("ConnectionModule init end()");
    }

    void ConnectionModule::terminate()
    {
        Logger::printfln("ConnectionModule::terminate()");
        this->sendChannel.unsubscribe();
        this->receiveChannel.unpublish();
        this->errorChannel.unpublish();
        Logger::printfln("ConnectionModule::terminate() end");
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
