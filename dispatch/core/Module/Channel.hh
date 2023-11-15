#pragma once

#include "dispatch/core/Module/ChannelAccessRights.hh"
#include "dispatch/core/Module/Publisher.hh"
#include "dispatch/core/Module/Subscriber.hh"

#include "dispatch/core/Module/ModuleRef.hh"

namespace claid {

template <typename T>
class Channel 
{
private:
    const std::string channelId;
    const ChannelAccessRights accessRights;
    ModuleRef parent;

    Publisher<T>* publisher;
    Subscriber<T>* subscriber;
    bool callbackRegistered = false;

    bool valid = false;

    bool canRead() const {
        return accessRights == ChannelAccessRights::READ || accessRights == ChannelAccessRights::READ_WRITE;
    }

    bool canWrite() const {
        return accessRights == ChannelAccessRights::WRITE || accessRights == ChannelAccessRights::READ_WRITE;
    }

     // Constructor for invalid Channels.
    explicit Channel(const std::string& channelId)
        : channelId(channelId), accessRights(ChannelAccessRights::NONE), parent(nullptr), valid(false) 
    {
       
    }

public:
    // Constructor for published Channels.
    Channel(ModuleRef parent, const std::string& channelId, Publisher<T>* publisher)
        : channelId(channelId), accessRights(ChannelAccessRights::WRITE), parent(parent), publisher(publisher), valid(true) 
    {

    }

    // Constructor for subscribed Channels.
    Channel(ModuleRef parent, const std::string& channelId, Subscriber<T>* subscriber)
        : channelId(channelId), accessRights(ChannelAccessRights::READ), parent(parent), subscriber(subscriber), valid(true) 
    {
        this->callbackRegistered = subscriber != nullptr;
    }


    static Channel<T> newInvalidChannel(const std::string& channelId) {
        return Channel<T>(channelId);
    }

    void post(T data) 
    {
        if (!canWrite()) 
        {
            std::string msg = " tried to post data to channel \"" + channelId + "\", however\n"
                "it did not publish this channel before.";
            parent.moduleError(msg);
            return;
        }
        publisher->post(data);
    }
};
}