package adamma.c4dhi.claid.Module;

import java.util.function.Consumer;

public class Channel<T>
{
    private final String channelId;
    private final ChannelAccessRights accessRights;
    private Module parent;

    private Consumer<T> callback;
    boolean callbackRegistered = false;

    private boolean canRead()
    {
        return this.accessRights == ChannelAccessRights.READ || this.accessRights == ChannelAccessRights.READ_WRITE;
    }

    private boolean canWrite()
    {
        return this.accessRights == ChannelAccessRights.WRITE || this.accessRights == ChannelAccessRights.READ_WRITE;
    }

    public Channel(Module parent, final String channelId, final ChannelAccessRights accessRights)
    {
        this.channelId = channelId;
        this.accessRights = accessRights;
    }

    public Channel(Module parent, final String channelId, final ChannelAccessRights accessRights, Consumer<T> callback)
    {
        this.channelId = channelId;
        this.accessRights = accessRights;
        this.callback = callback;
        this.callbackRegistered = callback != null;
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
    }
}