#include "RemoteConnection/RemoteModule/RemoteModule.hpp"
#include "RemoteConnection/Message/MessageData/MessageDataBinary.hpp"
#include "RemoteConnection/Message/MessageHeader/TestMessage.hpp"
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
        Logger::logInfo("SendMessageChannel post");
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

    void RemoteModule::setErrorChannel(Channel<Error> channel)
    {
        this->errorChannel = channel;
        this->localObserver->setErrorChannel(channel);
        this->remoteObserver->setErrorChannel(channel);
    }

    void RemoteModule::unpublishSendMessageChannel()
    {
        this->sendMessageChannel.unpublish();
    }

    void RemoteModule::unsubscribeReceiveMessageChannel()
    {
        this->receiveMessageChannel.unsubscribe();
    }

   void RemoteModule::unpublishErrorChannel()
   {
        this->errorChannel.unpublish();
   }

    void RemoteModule::initialize()
    {
        Logger::logInfo("RemoteModule::initialize()");
        // By using forkSubModuleInThread, the LocalObserver will run on the same thread as the RemoteModule (no extra overhead).
        this->localObserver = this->forkSubModuleInThread<LocalObserver>();
        this->remoteObserver = this->forkSubModuleInThread<RemoteObserver>(&CLAID_RUNTIME->channelManager);
        Logger::logInfo("RemoteModule::initialize() end");
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
        


        // Logger::logInfo("Initialize");
        // size_t numChannels = CLAID_RUNTIME->getNumChannels();
        // std::vector<std::string> channelNames = {"IntChannel", "CoughChannel", "TestChannel"};
        // Message message = Message::CreateMessage<MessageHeaderChannelUpdate, MessageDataBinary>();
        // message.data->as<MessageDataBinary>()->set<std::vector<std::string>>(channelNames);
        // this->sendMessage(message);

        //  for(size_t i = 0; i < numChannels; i++)
        // {
        //     channelNames.push_back(CLAID_RUNTIME->getChannelNameByIndex(i));
        // }

        // Logger::logInfo("RemoteModule cal sending.");

        this->localObserver->observe(&CLAID_RUNTIME->channelManager);
   //     this->registerPeriodicFunction("PeriodicTest", &RemoteModule::periodicTest, this, 1000);
        
    }

    // Get's called when stopModule() was called.
    void RemoteModule::terminate()
    {
        // At this point, the runnable dispatcher thread of this
        // module (and it's submodules) is stopped.
        // No other function will ever be executed in this thread.
        // Hence, now it is safe to unpublish and unsubscribe the channels,
        // since it can not happen that any further function scheduled for
        // the remote or local observer will be called, since the thread is stopped.
        // In other words: Thread is stopped, even if there is data in a channel that remote or
        // local observer have subscribed to, the associated callback will never be called.
        Logger::logInfo("RemoteModule terminating");
       

        this->removeSubModule(this->localObserver);
        this->removeSubModule(this->remoteObserver);
        this->sendMessageChannelSet = false;

        // Aaaand we are gone. Bye bye module.
    }

    void RemoteModule::stop()
    {
        if(this->isInitialized())
        {
            // Do NOT delete the sub modules just yet!
            // It could happen that there are still some callback functions scheduled for execution.
            // And because the submodules share the same thread with the RemoteModule, the dispatcher thread
            // is not stopped just yet. Therefore, even if the modules have been stopped now,
            // it could still happen that there are some unprocessed runnables in the queue that still might get executed.
            Logger::logInfo("Joining and removing local observer");
            this->stopSubModule(this->localObserver);
            Logger::logInfo("Joining and removing remote observer");
            this->stopSubModule(this->remoteObserver);
        }
    }

    
    void RemoteModule::periodicTest()
    {
        Logger::logInfo("Sending test message");
        Message message = Message::CreateMessage<TestMessage, MessageDataString>();
        this->sendMessage(message);
    }



}
}