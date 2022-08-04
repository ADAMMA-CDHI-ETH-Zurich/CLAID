#include "RemoteConnection/RemoteModule/RemoteModule.hpp"
#include "RemoteConnection/RemoteModule/LocalObserver.hpp"
#include "RemoteConnection/Message/MessageData/MessageDataBinary.hpp"

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
        if(!this->sendMessageChannelSet)
        {
            PORTAIBLE_THROW(Exception, "Error! RemoteModule tried to send a message, however a channel to send messages was never set."
            "Please make sure to call setSendMessageChannel of RemoteModule " << __FILE__ << " " << __LINE__);
        }
        Logger::printfln("SendMessageChannel post");
        this->sendMessageChannel.post(message);
    }

    void RemoteModule::setSendMessageChannel(Channel<Message> channel)
    {
        this->sendMessageChannelSet = true;
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

        std::vector<std::string> channelNames;
        Message m = message->value();
        m.data->as<MessageDataBinary>()->get<std::vector<std::string>>(channelNames);

        for(std::string name : channelNames)
        {
            Logger::printfln("Channel name: %s", name.c_str());
        }
    }    

    void RemoteModule::initialize()
    {
        
    
    }

    void RemoteModule::start()
    {
        if(!this->sendMessageChannelSet)
        {
            PORTAIBLE_THROW(Exception, "Error! RemoteModule started without calling setSendMessageChannel before!"
            "Please provide a channel for sending messages (i.e. Channel<RemoteConnection::Message> and call setSendMessageChannel "
            "prior to calling start")
        }
        // Why start not in initialize? 
        // Because setSendMessageChannel might not have been called yet.
        // RemoteModule is used with ConnectionLink and RemoteConnectedEntity,
        // which make sure to first link the RemoteModule to a ConnectionModule and 
        // then calling start afterwards.
        Logger::printfln("Initialize");
        size_t numChannels = PORTAIBLE_RUNTIME->getNumChannels();
        std::vector<std::string> channelNames = {"IntChannel", "CoughChannel", "TestChannel"};
        Message message = Message::CreateMessage<MessageHeader, MessageDataBinary>();
        message.data->as<MessageDataBinary>()->set<std::vector<std::string>>(channelNames);
        this->sendMessage(message);

         for(size_t i = 0; i < numChannels; i++)
        {
            channelNames.push_back(PORTAIBLE_RUNTIME->getChannelNameByIndex(i));
        }

        Logger::printfln("RemoteModule cal sending.");


        // By using forkSubModuleInThread, the LocalObserver will run on the same thread as the RemoteModule (no extra overhead).
        this->forkSubModuleInThread<LocalObserver>(this->sendMessageChannel);
    }

    




}
}