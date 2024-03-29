/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer
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

package adamma.c4dhi.claid.Module;

import java.util.function.Consumer;

import adamma.c4dhi.claid.Logger.Logger;

public class Channel<T>
{
    private final String channelId;
    private final ChannelAccessRights accessRights;
    private Module parent;

    private Publisher<T> publisher;
    private Subscriber<T> subscriber;
    boolean callbackRegistered = false;

    private boolean valid = false;

    private boolean canRead()
    {
        return this.accessRights == ChannelAccessRights.READ || this.accessRights == ChannelAccessRights.READ_WRITE;
    }

    private boolean canWrite()
    {
        return this.accessRights == ChannelAccessRights.WRITE || this.accessRights == ChannelAccessRights.READ_WRITE;
    }

    // Constructor for published Channels.
    public Channel(Module parent, final String channelId, final Publisher<T> publisher)
    {
        this.channelId = channelId;
        this.accessRights = ChannelAccessRights.WRITE;
        this.publisher = publisher;
        this.valid = true;
    }

    // Constructor for subscribed Chanenls.
    public Channel(Module parent, final String channelId, final Subscriber<T> subscriber)
    {
        this.channelId = channelId;
        this.accessRights = ChannelAccessRights.READ;
        this.subscriber = subscriber;
        this.callbackRegistered = subscriber != null;
        this.valid = true;
    }

    private Channel(final String channelId)
    {
        this.accessRights = ChannelAccessRights.NONE;
        this.channelId = channelId;
        this.parent = null;
        this.valid = false;
    }

    static<T> Channel<T> newInvalidChannel(final String channelId)
    {
        Channel<T> channel = new Channel<T>(channelId);
        return channel;
    }

    public void post(T data)
    {
        if(!canWrite())
        {
            String msg = " tried to post data to channel \"" + this.channelId + "\", however\n" +    
                        "it did not publish this channel before.";
            parent.moduleError(msg);
            return;
        }
        this.publisher.post(data, System.currentTimeMillis());
    }

    public void post(T data, long timestamp)
    {
        if(!canWrite())
        {
            String msg = " tried to post data to channel \"" + this.channelId + "\", however\n" +    
                        "it did not publish this channel before.";
            parent.moduleError(msg);
            return;
        }
        this.publisher.post(data, timestamp);
    }

    public String getChannelId() {
        return this.channelId;
    }


}