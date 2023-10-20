package adamma.c4dhi.claid.Module;

import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.Logger.SeverityLevel;
import adamma.c4dhi.claid.Module.Channel;

import java.util.Map;
import java.util.function.Consumer;

public abstract class Module
{
    protected String id = "unknown";

    private ChannelSubscriberPublisher subscriberPublisher;
    private boolean isInitializing = false;
    private boolean isInitialized = false;

    public Module()
    {
    }

    public void moduleError(final String error)
    {
        String errorMsg = "Module \"" + this.id + "\": " + error;
        Logger.log(SeverityLevel.ERROR, errorMsg);
    }

    protected void moduleWarning(final String warning)
    {
        String warningMsg = "Module \"" + this.id + "\": " + warning;
        Logger.log(SeverityLevel.WARNING, warningMsg);
    }

    public void runtimeInitialize(ChannelSubscriberPublisher subscriberPublisher, Map<String, String> properties)
    {
        if(this.isInitialized)
        {
            moduleError("Initialize called twice!");
            return;
        }
        this.subscriberPublisher = subscriberPublisher;
    
        // Publishing / subscribing is only allowed in initialize function.
        // Use booleans to guard the access to the publish/subscribe functions.
        this.isInitializing = true;
        this.initialize(properties);
        this.isInitializing = false;

        this.subscriberPublisher = null;
    }

    protected void initialize(Map<String, String> properties)
    {
        
    }

    public void setId(String id)
    {
        this.id = id;
    }

    public String getId()
    {
        return this.id;
    }

    protected<T> Channel<T> publish(Class<T> clz, final String channelName)
    {
        if(!this.isInitializing)
        {
            moduleError("Cannot publish channel \"" + channelName + "\". Publishing is only allowed during initialization (i.e., first call of the initialize function).");
            return Channel.newInvalidChannel(channelName);
        }
        return this.subscriberPublisher.publish(this, clz, channelName);
    }

    protected<T> Channel<T> subscribe(Class<T> clz, final String channelName, Consumer<T> callback)
    {
        if(!this.isInitializing)
        {
            moduleError("Cannot subscribe channel \"" + channelName + "\". Subscribing is only allowed during initialization (i.e., first call of the initialize function).");
            return Channel.newInvalidChannel(channelName);
        }
        return this.subscriberPublisher.subscribe(this, clz, channelName, callback);
    }
};