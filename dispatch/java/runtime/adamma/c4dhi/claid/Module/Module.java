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

    
    private boolean assertCanPublish(final String channelName)
    {
        if (!this.isInitializing) {
            moduleError("Cannot publish channel \"" + channelName + "\". Publishing is only allowed during initialization (i.e., the first call of the initialize function).");
            return false;
        }
        return true;
    }

    
    // Default publish method for all data types except of collections like ArrayList or Map.
    // providing the Class is sufficient for all data types except Collections (Collections are handled separately).
    protected<T> Channel<T> publish(final String channelName, Class<T> dataType)
    {
        if(!this.assertCanPublish(channelName))
        {
            return Channel.newInvalidChannel(channelName);
        }
        return this.subscriberPublisher.publish(this, new DataType(dataType), channelName);
    }

    // Alternative to the default publish method. Instead of providing an example instance,
    // Sadly, we cannot do this, because this function would also be called for Map or ArrayList types.
    // Thus, our only option is to use the Class<?> as data type for all classes and an instance T t for cases like Map and ArrayList.
    /*protected<T> Channel<T> publish(final String channelName, T t)
    {
        if(!this.assertCanPublish(channelName))
        {
            return Channel.newInvalidChannel(channelName);
        }
        Class<?> dataType = t.getClass();
        return this.subscriberPublisher.publish(this, new DataType(dataType), channelName);
    }*/

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

    protected Channel<ArrayList<Short>> publish(final String channelName, ArrayList<Short> shorts, ShortType... ignore) {
        if (!this.isInitializing) {
            moduleError("Cannot publish channel \"" + channelName + "\". Publishing is only allowed during initialization (i.e., the first call of the initialize function).");
            return Channel.newInvalidChannel(channelName);
        }
        return this.subscriberPublisher.publish(this, new DataType(ArrayList.class, "ArrayList<Short>"), channelName);
    }

    protected Channel<ArrayList<Integer>> publish(final String channelName, ArrayList<Integer> ints, IntegerType... ignore) {
        if (!this.isInitializing) {
            moduleError("Cannot publish channel \"" + channelName + "\". Publishing is only allowed during initialization (i.e., the first call of the initialize function).");
            return Channel.newInvalidChannel(channelName);
        }
        return this.subscriberPublisher.publish(this, new DataType(ArrayList.class, "ArrayList<Integer>"), channelName);
    }

    protected Channel<ArrayList<Long>> publish(final String channelName, ArrayList<Long> longs, LongType... ignore) {
        if (!this.isInitializing) {
            moduleError("Cannot publish channel \"" + channelName + "\". Publishing is only allowed during initialization (i.e., the first call of the initialize function).");
            return Channel.newInvalidChannel(channelName);
        }
        return this.subscriberPublisher.publish(this, new DataType(ArrayList.class, "ArrayList<Long>"), channelName);
    }

    protected Channel<ArrayList<Float>> publish(final String channelName, ArrayList<Float> floats, FloatType... ignore) {
        if (!this.isInitializing) {
            moduleError("Cannot publish channel \"" + channelName + "\". Publishing is only allowed during initialization (i.e., the first call of the initialize function).");
            return Channel.newInvalidChannel(channelName);
        }
        return this.subscriberPublisher.publish(this, new DataType(ArrayList.class, "ArrayList<Float>"), channelName);
    }

    protected Channel<ArrayList<Double>> publish(final String channelName, ArrayList<Double> dbs, DoubleType... ignore) {
        if (!this.isInitializing) {
            moduleError("Cannot publish channel \"" + channelName + "\". Publishing is only allowed during initialization (i.e., the first call of the initialize function).");
            return Channel.newInvalidChannel(channelName);
        }
        return this.subscriberPublisher.publish(this, new DataType(ArrayList.class, "ArrayList<Double>"), channelName);
    }

    protected Channel<ArrayList<String>> publish(final String channelName, ArrayList<String> strings, StringType... ignore) {
        if (!this.isInitializing) {
            moduleError("Cannot publish channel \"" + channelName + "\". Publishing is only allowed during initialization (i.e., the first call of the initialize function).");
            return Channel.newInvalidChannel(channelName);
        }
        return this.subscriberPublisher.publish(this, new DataType(ArrayList.class, "ArrayList<String>"), channelName);
    }
    
    // MAP
    protected Channel<Map<String, Short>> publish(final String channelName, Map<String, Short> shorts, ShortType... ignore) {
        if (!this.isInitializing) {
            moduleError("Cannot publish channel \"" + channelName + "\". Publishing is only allowed during initialization (i.e., the first call of the initialize function).");
            return Channel.newInvalidChannel(channelName);
        }
        return this.subscriberPublisher.publish(this, new DataType(Map.class, "Map<String, Short>"), channelName);
    }
    
    protected Channel<Map<String, Integer>> publish(final String channelName, Map<String, Integer> ints, IntegerType... ignore) {
        if (!this.isInitializing) {
            moduleError("Cannot publish channel \"" + channelName + "\". Publishing is only allowed during initialization (i.e., the first call of the initialize function).");
            return Channel.newInvalidChannel(channelName);
        }
        return this.subscriberPublisher.publish(this, new DataType(Map.class, "Map<String, Integer>"), channelName);
    }
    
    protected Channel<Map<String, Long>> publish(final String channelName, Map<String, Long> longs, LongType... ignore) {
        if (!this.isInitializing) {
            moduleError("Cannot publish channel \"" + channelName + "\". Publishing is only allowed during initialization (i.e., the first call of the initialize function).");
            return Channel.newInvalidChannel(channelName);
        }
        return this.subscriberPublisher.publish(this, new DataType(Map.class, "Map<String, Long>"), channelName);
    }
    
    protected Channel<Map<String, Float>> publish(final String channelName, Map<String, Float> floats, FloatType... ignore) {
        if (!this.isInitializing) {
            moduleError("Cannot publish channel \"" + channelName + "\". Publishing is only allowed during initialization (i.e., the first call of the initialize function).");
            return Channel.newInvalidChannel(channelName);
        }
        return this.subscriberPublisher.publish(this, new DataType(Map.class, "Map<String, Float>"), channelName);
    }
    
    protected Channel<Map<String, Double>> publish(final String channelName, Map<String, Double> dbs, DoubleType... ignore) {
        if (!this.isInitializing) {
            moduleError("Cannot publish channel \"" + channelName + "\". Publishing is only allowed during initialization (i.e., the first call of the initialize function).");
            return Channel.newInvalidChannel(channelName);
        }
        return this.subscriberPublisher.publish(this, new DataType(Map.class, "Map<String, Double>"), channelName);
    }
    
    protected Channel<Map<String, String>> publish(final String channelName, Map<String, String> strings, StringType... ignore) {
        if (!this.isInitializing) {
            moduleError("Cannot publish channel \"" + channelName + "\". Publishing is only allowed during initialization (i.e., the first call of the initialize function).");
            return Channel.newInvalidChannel(channelName);
        }
        return this.subscriberPublisher.publish(this, new DataType(Map.class, "Map<String, String>"), channelName);
    }
    
    
    

    // =================== Subscribe ===================
   
    private boolean assertCanSubscribe(final String channelName)
    {
        if(!this.isInitializing)
        {
            moduleError("Cannot subscribe channel \"" + channelName + "\". Subscribing is only allowed during initialization (i.e., first call of the initialize function).");
            return false;
        }
        return true;
    }

    /*protected<T> Channel<T> subscribe(final String channelName, T t, Consumer<ChannelData<T>> callback)
    {
        if(!this.assertCanSubscribe(channelName))
        {
            return Channel.newInvalidChannel(channelName);
        }

        Class<?> dataTypeClass = t.getClass();

        DataType dataType = new DataType(dataTypeClass);
        return this.subscriberPublisher.subscribe(this, dataType, channelName, new Subscriber<T>(dataType, callback, this.runnableDispatcher));
    }*/

    protected<T> Channel<T> subscribe(final String channelName, Class<T> dataTypeClass, Consumer<ChannelData<T>> callback)
    {
        if(!this.assertCanSubscribe(channelName))
        {
            return Channel.newInvalidChannel(channelName);
        }

        DataType dataType = new DataType(dataTypeClass);
        return this.subscriberPublisher.subscribe(this, dataType, channelName, new Subscriber<T>(dataType, callback, this.runnableDispatcher));
    }

    // =================== Subscribe ArrayList ===================


    protected<T> Channel<T> subscribe(final String channelName, ArrayList<Short> dbs, Consumer<ChannelData<T>> callback, ShortType ...ignore)
    {
        if(!this.assertCanSubscribe(channelName))
        {
            return Channel.newInvalidChannel(channelName);
        }

        DataType dataType = new DataType(ArrayList.class, "ArrayList<Short>");
        return this.subscriberPublisher.subscribe(this, dataType, channelName, new Subscriber<T>(dataType, callback, this.runnableDispatcher));
    }

    protected<T> Channel<T> subscribe(final String channelName, ArrayList<Integer> dbs, Consumer<ChannelData<T>> callback, IntegerType ...ignore)
    {
        if(!this.assertCanSubscribe(channelName))
        {
            return Channel.newInvalidChannel(channelName);
        }

        DataType dataType = new DataType(ArrayList.class, "ArrayList<Integer>");
        return this.subscriberPublisher.subscribe(this, dataType, channelName, new Subscriber<T>(dataType, callback, this.runnableDispatcher));
    }

    protected<T> Channel<T> subscribe(final String channelName, ArrayList<Long> dbs, Consumer<ChannelData<T>> callback, LongType ...ignore)
    {
        if(!this.assertCanSubscribe(channelName))
        {
            return Channel.newInvalidChannel(channelName);
        }

        DataType dataType = new DataType(ArrayList.class, "ArrayList<Long>");
        return this.subscriberPublisher.subscribe(this, dataType, channelName, new Subscriber<T>(dataType, callback, this.runnableDispatcher));
    }

    protected<T> Channel<T> subscribe(final String channelName, ArrayList<Float> dbs, Consumer<ChannelData<T>> callback, FloatType ...ignore)
    {
        if(!this.assertCanSubscribe(channelName))
        {
            return Channel.newInvalidChannel(channelName);
        }

        DataType dataType = new DataType(ArrayList.class, "ArrayList<Float>");
        return this.subscriberPublisher.subscribe(this, dataType, channelName, new Subscriber<T>(dataType, callback, this.runnableDispatcher));
    }

    protected<T> Channel<T> subscribe(final String channelName, ArrayList<Double> dbs, Consumer<ChannelData<T>> callback, DoubleType ...ignore)
    {
        if(!this.assertCanSubscribe(channelName))
        {
            return Channel.newInvalidChannel(channelName);
        }

        DataType dataType = new DataType(ArrayList.class, "ArrayList<Double>");
        return this.subscriberPublisher.subscribe(this, dataType, channelName, new Subscriber<T>(dataType, callback, this.runnableDispatcher));
    }


    // =================== Subscribe Map ===================

    protected Channel<Map<String, Short>> subscribe(final String channelName, Map<String, Short> map, Consumer<ChannelData<Map<String, Short>>> callback, ShortType... ignore) {
        if (!this.assertCanSubscribe(channelName)) {
            return Channel.newInvalidChannel(channelName);
        }
    
        DataType dataType = new DataType(Map.class, "Map<String, Short>");
        return this.subscriberPublisher.subscribe(this, dataType, channelName, new Subscriber<>(dataType, callback, this.runnableDispatcher));
    }
    
    protected Channel<Map<String, Integer>> subscribe(final String channelName, Map<String, Integer> map, Consumer<ChannelData<Map<String, Integer>>> callback, IntegerType... ignore) {
        if (!this.assertCanSubscribe(channelName)) {
            return Channel.newInvalidChannel(channelName);
        }
    
        DataType dataType = new DataType(Map.class, "Map<String, Integer>");
        return this.subscriberPublisher.subscribe(this, dataType, channelName, new Subscriber<>(dataType, callback, this.runnableDispatcher));
    }
    
    protected Channel<Map<String, Long>> subscribe(final String channelName, Map<String, Long> map, Consumer<ChannelData<Map<String, Long>>> callback, LongType... ignore) {
        if (!this.assertCanSubscribe(channelName)) {
            return Channel.newInvalidChannel(channelName);
        }
    
        DataType dataType = new DataType(Map.class, "Map<String, Long>");
        return this.subscriberPublisher.subscribe(this, dataType, channelName, new Subscriber<>(dataType, callback, this.runnableDispatcher));
    }
    
    protected Channel<Map<String, Float>> subscribe(final String channelName, Map<String, Float> map, Consumer<ChannelData<Map<String, Float>>> callback, FloatType... ignore) {
        if (!this.assertCanSubscribe(channelName)) {
            return Channel.newInvalidChannel(channelName);
        }
    
        DataType dataType = new DataType(Map.class, "Map<String, Float>");
        return this.subscriberPublisher.subscribe(this, dataType, channelName, new Subscriber<>(dataType, callback, this.runnableDispatcher));
    }
    
    protected Channel<Map<String, Double>> subscribe(final String channelName, Map<String, Double> map, Consumer<ChannelData<Map<String, Double>>> callback, DoubleType... ignore) {
        if (!this.assertCanSubscribe(channelName)) {
            return Channel.newInvalidChannel(channelName);
        }
    
        DataType dataType = new DataType(Map.class, "Map<String, Double>");
        return this.subscriberPublisher.subscribe(this, dataType, channelName, new Subscriber<>(dataType, callback, this.runnableDispatcher));
    }
    
    protected Channel<Map<String, String>> subscribe(final String channelName, Map<String, String> map, Consumer<ChannelData<Map<String, String>>> callback, StringType... ignore) {
        if (!this.assertCanSubscribe(channelName)) {
            return Channel.newInvalidChannel(channelName);
        }
    
        DataType dataType = new DataType(Map.class, "Map<String, String>");
        return this.subscriberPublisher.subscribe(this, dataType, channelName, new Subscriber<>(dataType, callback, this.runnableDispatcher));
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