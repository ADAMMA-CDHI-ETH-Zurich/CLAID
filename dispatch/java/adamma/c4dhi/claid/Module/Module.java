package adamma.c4dhi.claid.Module;

import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.Logger.SeverityLevel;
import adamma.c4dhi.claid.Module.Channel;
import adamma.c4dhi.claid.DataPackage;

import java.util.Map;
import java.util.function.Consumer;

public abstract class Module
{
    protected String id = "unknown";

    private ChannelSubscriberPublisher subscriberPublisher;
    
    private RunnableDispatcher runnableDispatcher = new RunnableDispatcher();

    private boolean isInitializing = false;
    private boolean isInitialized = false;



    public Module(/*ThreadSafeChannel<DataPackage> moduleInputQueue,  
        ThreadSafeChannel<DataPackage> moduleOutputQueue*/)
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

    public boolean start(ChannelSubscriberPublisher subscriberPublisher, Map<String, String> properties)
    {
        if(this.isInitialized)
        {
            moduleError("Initialize called twice!");
            return false;
        }
        this.subscriberPublisher = subscriberPublisher;

        if(!this.runnableDispatcher.start())
        {
            moduleError("Failed to start RunnableDispatcher.");
            return false;
        }
 
        // Publishing / subscribing is only allowed in initialize function.
        // Use booleans to guard the access to the publish/subscribe functions.
        this.isInitializing = true;

        this.runnableDispatcher.addRunnable(new Runnable() {
            public void run(){
                initializeInternal(properties);
              }
        });

        while(!this.isInitialized)
        {
            try {
                Thread.sleep(20);
            } catch (InterruptedException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
                moduleError("Failed to wait for initialize function to finish: " + e.getMessage());
                return false;
            }
        }

        this.isInitializing = false;

        this.subscriberPublisher = null;
        return true;
    }

    protected void initializeInternal(Map<String, String> properties)
    {
        this.initialize(properties);
        this.isInitialized = true;
    }

    protected void initialize(Map<String, String> properties)
    {
        System.out.println("Module base initialize");
    }

    public void setId(String id)
    {
        this.id = id;
    }

    public String getId()
    {
        return this.id;
    }

    public void enqueueRunnable(Runnable runnable)
    {
        this.runnableDispatcher.addRunnable(runnable);
    }

    protected<T> Channel<T> publish(Class<T> dataType, final String channelName)
    {
        if(!this.isInitializing)
        {
            moduleError("Cannot publish channel \"" + channelName + "\". Publishing is only allowed during initialization (i.e., first call of the initialize function).");
            return Channel.newInvalidChannel(channelName);
        }
        return this.subscriberPublisher.publish(this, dataType, channelName);
    }

    protected<T> Channel<T> subscribe(Class<T> dataType, final String channelName, Consumer<T> callback)
    {
        if(!this.isInitializing)
        {
            moduleError("Cannot subscribe channel \"" + channelName + "\". Subscribing is only allowed during initialization (i.e., first call of the initialize function).");
            return Channel.newInvalidChannel(channelName);
        }
        return this.subscriberPublisher.subscribe(this, dataType, channelName, new Subscriber<T>(dataType, callback, this.runnableDispatcher));
    }
};