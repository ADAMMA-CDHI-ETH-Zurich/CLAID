package adamma.c4dhi.claid.Module;

import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.Logger.SeverityLevel;
import adamma.c4dhi.claid.Module.Scheduling.FunctionRunnable;
import adamma.c4dhi.claid.Module.Scheduling.FunctionRunnable;
import adamma.c4dhi.claid.Module.Scheduling.RunnableDispatcher;
import adamma.c4dhi.claid.Module.Scheduling.ScheduleOnce;
import adamma.c4dhi.claid.Module.Scheduling.ScheduleRepeatedIntervall;
import adamma.c4dhi.claid.Module.Scheduling.ScheduledRunnable;
import adamma.c4dhi.claid.TypeMapping.DataType;
import adamma.c4dhi.claid.DataPackage;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.function.Consumer;
import java.time.Duration;
import java.time.LocalDateTime;

public abstract class Module
{
    protected String id = "unknown";

    private ChannelSubscriberPublisher subscriberPublisher;
    
    private RunnableDispatcher runnableDispatcher = new RunnableDispatcher();

    private boolean isInitializing = false;
    private boolean isInitialized = false;

    Map<String, ScheduledRunnable> timers = new HashMap<>();



    public Module(/*ThreadSafeChannel<DataPackage> moduleInputQueue,  
        ThreadSafeChannel<DataPackage> moduleOutputQueue*/)
    {
    }

    public void moduleError(final String error)
    {
        String errorMsg = "Module \"" + this.id + "\": " + error;
        Logger.log(SeverityLevel.ERROR, errorMsg);
        System.exit(0);
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

        this.runnableDispatcher.addRunnable(new FunctionRunnable(() -> initializeInternal(properties)));

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

    private void enqueueRunnable(ScheduledRunnable runnable)
    {
        this.runnableDispatcher.addRunnable(runnable);

    }

    
    // Default publish method for all data types except of collections like ArrayList or Map.
    protected<T> Channel<T> publish(Class<T> dataType, final String channelName)
    {
        if(!this.isInitializing)
        {
            moduleError("Cannot publish channel \"" + channelName + "\". Publishing is only allowed during initialization (i.e., first call of the initialize function).");
            return Channel.newInvalidChannel(channelName);
        }
        return this.subscriberPublisher.publish(this, new DataType(dataType), channelName);
    }
    
    // Magic to tackle java type erasure, because there is absolutely no way
    // to find out the generic type of a collection from the Class object (e.g., new ArrayList<String>().class()) itself.
    // We require an instance of the Collection and use some magic to trick the compiler into generating
    // override functions for different types of the collection. Please cechkout manual for wizardry 74411632: https://stackoverflow.com/a/74411632
    public static class ShortType {}
    public static class IntegerType {}
    public static class LongType {}
    public static class FloatType {}
    public static class DoubleType {}
    public static class StringType {}

    protected Channel<ArrayList<Short>> publish(ArrayList<Short> shorts, final String channelName, ShortType... ignore) {
        System.out.println("Hello, Short!");
        if (!this.isInitializing) {
            moduleError("Cannot publish channel \"" + channelName + "\". Publishing is only allowed during initialization (i.e., the first call of the initialize function).");
            return Channel.newInvalidChannel(channelName);
        }
        return this.subscriberPublisher.publish(this, new DataType(ArrayList.class, "ArrayList<Short>"), channelName);
    }

    protected Channel<ArrayList<Integer>> publish(ArrayList<Integer> ints, final String channelName, IntegerType... ignore) {
        System.out.println("Hello, Integer!");
        if (!this.isInitializing) {
            moduleError("Cannot publish channel \"" + channelName + "\". Publishing is only allowed during initialization (i.e., the first call of the initialize function).");
            return Channel.newInvalidChannel(channelName);
        }
        return this.subscriberPublisher.publish(this, new DataType(ArrayList.class, "ArrayList<Integer>"), channelName);
    }

    protected Channel<ArrayList<Long>> publish(ArrayList<Long> longs, final String channelName, LongType... ignore) {
        System.out.println("Hello, Long!");
        if (!this.isInitializing) {
            moduleError("Cannot publish channel \"" + channelName + "\". Publishing is only allowed during initialization (i.e., the first call of the initialize function).");
            return Channel.newInvalidChannel(channelName);
        }
        return this.subscriberPublisher.publish(this, new DataType(ArrayList.class, "ArrayList<Long>"), channelName);
    }

    protected Channel<ArrayList<Float>> publish(ArrayList<Float> floats, final String channelName, FloatType... ignore) {
        System.out.println("Hello, Float!");
        if (!this.isInitializing) {
            moduleError("Cannot publish channel \"" + channelName + "\". Publishing is only allowed during initialization (i.e., the first call of the initialize function).");
            return Channel.newInvalidChannel(channelName);
        }
        return this.subscriberPublisher.publish(this, new DataType(ArrayList.class, "ArrayList<Float>"), channelName);
    }

    protected Channel<ArrayList<Double>> publish(ArrayList<Double> dbs, final String channelName, DoubleType... ignore) {
        System.out.println("Hello, Double!");
        if (!this.isInitializing) {
            moduleError("Cannot publish channel \"" + channelName + "\". Publishing is only allowed during initialization (i.e., the first call of the initialize function).");
            return Channel.newInvalidChannel(channelName);
        }
        return this.subscriberPublisher.publish(this, new DataType(ArrayList.class, "ArrayList<Double>"), channelName);
    }

    protected Channel<ArrayList<String>> publish(ArrayList<String> strings, final String channelName, StringType... ignore) {
        System.out.println("Hello, String!");
        if (!this.isInitializing) {
            moduleError("Cannot publish channel \"" + channelName + "\". Publishing is only allowed during initialization (i.e., the first call of the initialize function).");
            return Channel.newInvalidChannel(channelName);
        }
        return this.subscriberPublisher.publish(this, new DataType(ArrayList.class, "ArrayList<String>"), channelName);
    }
    
   

   

    protected<T> Channel<T> subscribe(Class<T> dataTypeClass, final String channelName, Consumer<T> callback)
    {
        if(!this.isInitializing)
        {
            moduleError("Cannot subscribe channel \"" + channelName + "\". Subscribing is only allowed during initialization (i.e., first call of the initialize function).");
            return Channel.newInvalidChannel(channelName);
        }

        DataType dataType = new DataType(dataTypeClass);
        return this.subscriberPublisher.subscribe(this, dataType, channelName, new Subscriber<T>(dataType, callback, this.runnableDispatcher));
    }

    protected<T> Channel<T> subscribe(ArrayList<Double> dbs, final String channelName, Consumer<T> callback, DoubleType ...ignore)
    {
        if(!this.isInitializing)
        {
            moduleError("Cannot subscribe channel \"" + channelName + "\". Subscribing is only allowed during initialization (i.e., first call of the initialize function).");
            return Channel.newInvalidChannel(channelName);
        }

        DataType dataType = new DataType(ArrayList.class, "ArrayList<Double>");
        return this.subscriberPublisher.subscribe(this, dataType, channelName, new Subscriber<T>(dataType, callback, this.runnableDispatcher));
    }

    protected void registerPeriodicFunction(final String name, Runnable callback, Duration intervall)
    {
        this.registerPeriodicFunction(name, callback, intervall, LocalDateTime.now().plus(intervall));
    }

    protected void registerPeriodicFunction(final String name, Runnable callback, Duration intervall, LocalDateTime startTime)
    {
        
        if (intervall.toMillis() == 0) 
        {
            this.moduleError("Error in registerPeriodicFunction: Cannot register periodic function \"" + name + "\" with a period of 0 milliseconds.");
        }

        if (timers.containsKey(name))
        {
            this.moduleError("Tried to register function with name \"" + name + "\", but a periodic function with the same name was already registered before.");
        }

        FunctionRunnable runnable = new FunctionRunnable(callback, new ScheduleRepeatedIntervall(startTime, intervall));

       
        timers.put(name, runnable);
        this.runnableDispatcher.addRunnable(runnable);
    }


    protected void unregisterPeriodicFunction(final String name)
    {
        if(!timers.containsKey(name))
        {
            moduleError("Error, tried to unregister periodic function but function was not found in list of registered timers."
            + "Was a function with this name ever registered before?");
        }
        
        timers.get(name).invalidate();
        timers.remove(name);
    }

    protected void registerScheduledFunction(final String name, LocalDateTime dateTime, Runnable function)
    {   
        if(dateTime.isBefore(LocalDateTime.now()))
        {
            moduleWarning("Failed to schedule function \"" + name + "\" at time " + dateTime + ". The time is in the past. It is now: " + LocalDateTime.now());
        }

        FunctionRunnable runnable = new FunctionRunnable(function, new ScheduleOnce(dateTime));
        timers.put(name, runnable);
    }


//         String name, Duration period, RegisteredCallback callback) =>
//     _scheduler.registerPeriodicFunction(_modId, name, period, callback);

// void unregisterPeriodicFunction(String name) =>
//     _scheduler.unregisterPeriodicFunction(_modId, name);

// void registerScheduledFunction(
//         String name, DateTime dateTime, RegisteredCallback callback) =>
//     _scheduler.registerScheduledFunction(_modId, name, dateTime, callback);

// void unregisterScheduledFunction(String name) =>
//     _scheduler.unregisterScheduledFunction(_modId, name);
};