#pragma once

#include "RunTime/RunTime.hpp"
#include "RemoteConnection/Message/Message.hpp"
namespace portaible
{
    namespace RemoteConnection
    {
        // A RemoteModule is a module that mirrors all Modules running in another RunTime (remotely).
        // Whenever a channel is published/subscribed or data is posted by
        // in the remote runtime, the RemoteModule will perform the same actions locally.
        class RemoteModule : public Module
        {
            private:
                
                // Channels that were published remotely
                std::vector<Channel<Untyped>> remotePublishedChannels;

                // Channels that were subscribed remotely.
                std::vector<Channel<Untyped>> remoteSubscribedChannels;

                Channel<Message> sendMessageChannel;
                Channel<Message> receiveMessageChannel;

                void onMessageReceived(ChannelData<Message> message);
                void sendMessage(Message& message);
            //     // Observes everything the remote connection does.
            //     class RemoteObserver 
            //     {
            //         // Receives data from network and is also able to send data over network ?
            //         void onRemoteModuleAdded();
            //         void onRemoteModuleRemoved();
            //         void onRemoteModuleSubscribedToChannel();
            //         void onRemoteModulePublishedChannel();
            //     };

            //     // Observes everything we do locally.
            //     class LocalObserver 
            //     {
            //         // Does not receive data from network, but is able to send data over network.

            //         void onLocalModuleAdded();
            //         void onLocalModuleRemoved();
            //         void onLocalModuleSubcribedToChannel();
            //         void onLocalModulePublishedChannel();
            //     };

            //     void onMessage(Message message);
            //     void onConnectionLost();

            public:
            //     void notifyOnMessage(Message message);
            //     void notifyOnConnectionLost();
                ChannelSubscriber<Message> getSubscriberForMessageReceiveChannel();

                void setSendMessageChannel(Channel<Message> channel);
                void setReceiveMessageChannel(Channel<Message> channel);
                
                void unpublishSendMessageChannel();
                void unsubscribeReceiveMessageChannel();
            
        };
    }
    
}

// void onDataReceived()
// {
//     onRemotePublish();
//     onRemoteSubscribe();
//     onRemoteModuleAdded();
//     onRemoteModuleRemoved();
//     onRemoteData();
// }


// void onRemotePublish(std::string& channelID)
// {
//     // A channel got published in the remote runtime.
//     // We publish it aswell? 
//     // How many times? Only once or 
//     // for every time it got published in the remote framework.
//     // The latter is useful if we want to know how many subscribers a channel has.

// }

// void onRemoteSubscribe(std::string& channelID)
// {
//     // Someone in the remote runtime subscribed to a channel.
//     // Thus, we subscribe. When data is available (callback was called).
//     // What about the history... do we need to send it ? Nahhhhh not for now

//     // TODO: Think about whether history (previous data in channel) needs to be send.
//     Channel<Untyped> remoteSubscribedChannel = this->subscribe<Untyped>(channelID, &RemoteModule::onLocalModulePostedData, this);
//     this->remoteSubscribedChannels.push_back(remoteSubscribedChannel);
// }

// void onLocalModulePostedData(ChannelData<Untyped> data)
// { 
//     // Someone in the current runtime/process posted data to a channel, that
//     // was subscribed to in the remote Runtime. Thus, we need to send the data
//     // to the remote runtime.
// }


// void onLocalPublish()
// {
//     // Send to other runtime that new channel was published
// }

// void onLocalSubscribe()
// {
//     // Tell the remote module in the other thread, that we subscribed.
// }

// void onLocalModuleAdded()
// {

// }

// void onLocalModuleRemoved()
// {

// }

// void onPublish(const std::string& channelID)
// {
//     // Publish means that the Module running in the remote RunTime, that we mirror,
//     // published a channel. Thus, we need to publish it aswell..
//     // How do we do that?
//     Channel<Untyped> = this->publish<Untyped>(channelID);
// }