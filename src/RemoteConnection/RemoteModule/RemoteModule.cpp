#include "RemoteConnection/RemoteModule/RemoteModule.hpp"

#include "RemoteConnection/Message/MessageDataBinary.hpp"

namespace portaible
{
namespace RemoteConnection
{
    //     void notifyOnConnectionLost();
    ChannelSubscriber<Message> RemoteModule::getSubscriberForMessageReceiveChannel()
    {
        return this->makeSubscriber(&RemoteModule::onMessageReceived, this);
    }

    void RemoteModule::sendMessage(Message& message)
    {
        Logger::printfln("SendMessageChannel post");
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

    void RemoteModule::unpublishSendMessageChannel()
    {
        this->sendMessageChannel.unpublish();
    }

    void RemoteModule::unsubscribeReceiveMessageChannel()
    {
        this->receiveMessageChannel.unsubscribe();
    }

    void RemoteModule::onMessageReceived(ChannelData<Message> message)
    {
        Logger::printfln("RECEIVED MESSAGE\n");
        Logger::printfln("%s", message->value().header->description.c_str());
    }    

    void RemoteModule::initialize()
    {
        size_t numChannels = PORTAIBLE_RUNTIME->getNumChannels();
        std::vector<std::string> channelNames;

        for(size_t i = 0; i < numChannels; i++)
        {
            channelNames.push_back(PORTAIBLE_RUNTIME->getChannelNameByIndex(i));
        }
        Logger::printfln("RemoteModule sending.");
        Message message = Message::CreateMessage<MessageHeader, MessageDataBinary>();
                Logger::printfln("RemoteModule sending 2.");

        message.header->description = "This is a test";
                Logger::printfln("RemoteModule sending 3.");

        message.data->as<MessageDataBinary>()->set<std::vector<std::string>>(channelNames);
                Logger::printfln("RemoteModule sending 4");

        Logger::printfln("RemoteModule cal sending.");

        this->sendMessage(message);
    }




}
}