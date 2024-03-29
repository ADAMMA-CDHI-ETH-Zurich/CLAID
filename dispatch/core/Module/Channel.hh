/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Stephan Altmüller
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*         http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
***************************************************************************/

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
    std::string channelId;
    ChannelAccessRights accessRights;
    ModuleRef parent;

    std::shared_ptr<Publisher<T>> publisher;
    std::shared_ptr<Subscriber<T>> subscriber;
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

    Channel() : accessRights(ChannelAccessRights::NONE), parent(nullptr)
    {
        this->valid = false;
    }

    // Constructor for published Channels.
    Channel(ModuleRef parent, const std::string& channelId, std::shared_ptr<Publisher<T>> publisher)
        : channelId(channelId), accessRights(ChannelAccessRights::WRITE), parent(parent), publisher(publisher), valid(true) 
    {

    }

    // Constructor for subscribed Channels.
    Channel(ModuleRef parent, const std::string& channelId, std::shared_ptr<Subscriber<T>> subscriber)
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