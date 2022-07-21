#include "RemoteConnection/RemoteModule/RemoteModule.hpp"


namespace portaible
{
namespace RemoteConnection
{
    //     void notifyOnConnectionLost();
    ChannelSubscriber<Message> RemoteModule::getSubscriberForMessageReceiveChannel()
    {
        return this->makeSubscriber(&RemoteModule::onMessageReceived, this);
    }

    void RemoteModule::onMessageReceived(ChannelData<Message> message)
    {
        
    }    

    void RemoteModule::sendMessage(Message& message)
    {
        this->sendMessageChannel.post(message);
    }

    void RemoteModule::setSendMessageChannel(Channel<Message> channel)
    {
        this->sendMessageChannel = channel;
    }

    void RemoteModule::setReceiveMessageChannel(Channel<Message> channel)
    {
        this->receiveMessageChannel = channel;
    }




}
}