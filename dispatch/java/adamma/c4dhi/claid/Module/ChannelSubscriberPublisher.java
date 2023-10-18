package adamma.c4dhi.claid.Module;

import java.util.function.Consumer;

import adamma.c4dhi.claid.LocalDispatching.ModuleDispatcher;

public class ChannelSubscriberPublisher 
{

    public ChannelSubscriberPublisher()
    {
    }

    protected<T> Channel<T> publish(Module module, Class<T> clz, final String channelName)
    {
   
        return new Channel<T>(module, channelName, ChannelAccessRights.WRITE);
    }

    protected<T> Channel<T> subscribe(Module module ,Class<T> clz, final String channelName, Consumer<T> callback)
    {

        return new Channel<T>(module, channelName, ChannelAccessRights.READ, callback);
    }

}
