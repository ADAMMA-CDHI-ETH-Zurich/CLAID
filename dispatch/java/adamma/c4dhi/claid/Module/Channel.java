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
            String msg = "Module \"" + this.parent.getId() + "\"] tried to post data to channel \"" + this.channelId + "\", however\n" +    
                        "it did not publish this channel before.";
            parent.moduleError(msg);
            return;
        }
        this.publisher.post(data);
    }
}