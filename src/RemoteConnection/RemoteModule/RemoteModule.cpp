#include "RemoteConnection/RemoteModule/RemoteModule.hpp"
#include "RemoteConnection/Message/MessageData/MessageDataBinary.hpp"

namespace claid
{
namespace RemoteConnection
{
    ChannelSubscriber<Message> RemoteModule::getSubscriberForReceptionOfMessages()
    {
        if(this->remoteObserver == nullptr)
        {
            CLAID_THROW(Exception, "Error in RemoteModule: getSubscriberForReceptionOfMessages() was called before "
            "the RemoteModule has been initialized. Please call startModule() first");
        }

        return this->remoteObserver->getSubscriberForReceptionOfMessages();
    }

    void RemoteModule::sendMessage(Message& message)
    {
        if(!this->sendMessageChannelSet)
        {
            CLAID_THROW(Exception, "Error! RemoteModule tried to send a message, however a channel to send messages was never set."
            "Please make sure to call setSendMessageChannel of RemoteModule " << __FILE__ << " " << __LINE__);
        }
        Logger::printfln("SendMessageChannel post");
        this->sendMessageChannel.post(message);
    }

    void RemoteModule::setSendMessageChannel(Channel<Message> channel)
    {
        if(this->localObserver == nullptr || this->remoteObserver == nullptr)
        {
            CLAID_THROW(Exception, "Error, RemoteModule::setSendMessageChannel called before RemoteModule has been initialized. "
            << "Please initialize RemoteModule before setting the channel used to send messages");
        }

        this->localObserver->setSendMessageChannel(channel);
        this->remoteObserver->setSendMessageChannel(channel);
        
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

   

    void RemoteModule::initialize()
    {
        // By using forkSubModuleInThread, the LocalObserver will run on the same thread as the RemoteModule (no extra overhead).
        this->localObserver = this->forkSubModuleInThread<LocalObserver>();
        this->remoteObserver = this->forkSubModuleInThread<RemoteObserver>(&CLAID_RUNTIME->channelManager);
    }


    // Why "start" not in initialize? 
    // Because setSendMessageChannel might not have been called yet.
    // RemoteModule is used with ConnectionLink and RemoteConnectedEntity,
    // which make sure to first link the RemoteModule to a ConnectionModule and 
    // then calling start afterwards.
    void RemoteModule::start()
    {
        if(!this->sendMessageChannelSet)
        {
            CLAID_THROW(Exception, "Error! RemoteModule started without calling setSendMessageChannel before!"
            "Please provide a channel for sending messages (i.e. Channel<RemoteConnection::Message> and call setSendMessageChannel "
            "prior to calling start");
        }
        


        // Logger::printfln("Initialize");
        // size_t numChannels = CLAID_RUNTIME->getNumChannels();
        // std::vector<std::string> channelNames = {"IntChannel", "CoughChannel", "TestChannel"};
        // Message message = Message::CreateMessage<MessageHeaderChannelUpdate, MessageDataBinary>();
        // message.data->as<MessageDataBinary>()->set<std::vector<std::string>>(channelNames);
        // this->sendMessage(message);

        //  for(size_t i = 0; i < numChannels; i++)
        // {
        //     channelNames.push_back(CLAID_RUNTIME->getChannelNameByIndex(i));
        // }

        // Logger::printfln("RemoteModule cal sending.");

        this->localObserver->observe(&CLAID_RUNTIME->channelManager);
        
    }

    // Get's called when stopModule() was called.
    void RemoteModule::terminate()
    {
        this->unpublishSendMessageChannel();
        this->unsubscribeReceiveMessageChannel();
        this->sendMessageChannelSet = false;
    }

    void RemoteModule::stop()
    {
        if(this->isInitialized())
        {
            this->joinAndRemoveSubModule(this->localObserver);
            this->joinAndRemoveSubModule(this->remoteObserver);
        }
    }

    




}
}