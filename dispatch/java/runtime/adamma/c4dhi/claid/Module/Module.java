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

import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.Module.Scheduling.FunctionRunnable;
import adamma.c4dhi.claid.Module.Scheduling.FunctionRunnable;
import adamma.c4dhi.claid.Module.Scheduling.RunnableDispatcher;
import adamma.c4dhi.claid.Module.Scheduling.ScheduleHelper;
import adamma.c4dhi.claid.Module.Scheduling.ScheduleOnce;
import adamma.c4dhi.claid.Module.Scheduling.ScheduleRepeatedIntervall;
import adamma.c4dhi.claid.Module.Scheduling.ScheduledRunnable;
import adamma.c4dhi.claid.RemoteFunction.RemoteFunction;
import adamma.c4dhi.claid.Runtime;

import adamma.c4dhi.claid.EventTracker.EventTracker;

import adamma.c4dhi.claid.Schedule;
import adamma.c4dhi.claid.ScheduleExactTime;
import adamma.c4dhi.claid.SchedulePeriodic;
import adamma.c4dhi.claid.ScheduleTimeWindow;
import adamma.c4dhi.claid.TypeMapping.DataType;
import adamma.c4dhi.claid.DataPackage;
import adamma.c4dhi.claid.LogMessageSeverityLevel;
import adamma.c4dhi.claid.LogMessageEntityType;
import adamma.c4dhi.claid.PowerProfile;
import adamma.c4dhi.claid.Module.Properties;
import adamma.c4dhi.claid.RemoteFunction.Future;


import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.function.Consumer;


import java.time.Duration;
import java.time.LocalDateTime;

import adamma.c4dhi.claid.Module.Scheduling.ConsumerRunnable;
import adamma.c4dhi.claid.Module.Scheduling.RunnableDispatcher;
import adamma.c4dhi.claid.Module.Scheduling.ScheduleOnce;
import adamma.c4dhi.claid.Module.Scheduling.ScheduledRunnable;
import adamma.c4dhi.claid.RemoteFunction.RemoteFunctionRunnable;

import adamma.c4dhi.claid.RemoteFunction.RemoteFunctionHandler;
import adamma.c4dhi.claid.RemoteFunction.RemoteFunctionRunnableHandler;

public abstract class Module
{
    protected String id = "unknown";
    protected String type = "unknown";
    protected EventTracker eventTracker;

    protected ChannelSubscriberPublisher subscriberPublisher;
    protected RemoteFunctionHandler remoteFunctionHandler;
    private RemoteFunctionRunnableHandler remoteFunctionRunnableHandler = null;

    private RunnableDispatcher runnableDispatcher = new RunnableDispatcher();

    private boolean isInitializing = false;
    private boolean isInitialized = false;
    private boolean isTerminating = false;
    private boolean isTerminated = false;
    private boolean isPaused = false;

    private RemoteFunction<Boolean> isConnectedToRemoteServerRemoteFunction;

    Map<String, ScheduledRunnable> timers = new HashMap<>();

    // Functions registered by this Module, which can be executed from other Modules or entities.
    Map<String, RemoteFunctionRunnable> registeredRemoteFunctionRunnables = new HashMap<>();

    public Module(/*ThreadSafeChannel<DataPackage> moduleInputQueue,  
        ThreadSafeChannel<DataPackage> moduleOutputQueue*/)
    {
    }

    public void moduleFatal(final String error)
    {
        String errorMsg = "Module \"" + this.id + "\": " + error;
        Logger.log(LogMessageSeverityLevel.FATAL, errorMsg, LogMessageEntityType.MODULE, this.id);
        throw new RuntimeException(errorMsg);
    }

    public void moduleError(final String error)
    {
        String errorMsg = "Module \"" + this.id + "\": " + error;
        Logger.log(LogMessageSeverityLevel.ERROR, errorMsg, LogMessageEntityType.MODULE, this.id);
        //throw new RuntimeException(errorMsg);
    }

    protected void moduleWarning(final String warning)
    {
        String warningMsg = "Module \"" + this.id + "\": " + warning;
        Logger.log(LogMessageSeverityLevel.WARNING, warningMsg, LogMessageEntityType.MODULE, this.id);
    }

    protected void moduleInfo(final String info)
    {
        String infoMsg = "Module \"" + this.id + "\": " + info;
        Logger.log(LogMessageSeverityLevel.INFO, infoMsg, LogMessageEntityType.MODULE, this.id);
    }

    protected void moduleDebug(final String debug)
    {
        String debugMsg = "Module \"" + this.id + "\": " + debug;
        Logger.log(LogMessageSeverityLevel.WARNING, debugMsg, LogMessageEntityType.MODULE, this.id);
    }

    public boolean start(ChannelSubscriberPublisher subscriberPublisher, RemoteFunctionHandler remoteFunctionHandler, Properties properties)
    {
        if(this.isInitialized)
        {
            moduleError("Initialize called twice!");
            return false;
        }
        this.subscriberPublisher = subscriberPublisher;
        this.remoteFunctionHandler = remoteFunctionHandler;
        this.remoteFunctionRunnableHandler = new RemoteFunctionRunnableHandler("Module " + this.id, subscriberPublisher.getToModuleManagerQueue());

        this.isConnectedToRemoteServerRemoteFunction = 
            remoteFunctionHandler.mapRuntimeFunction(Runtime.MIDDLEWARE_CORE, "is_connected_to_remote_server", null);


        this.runnableDispatcher.setRemoteFunctionHandler(this.remoteFunctionHandler);
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

    public boolean shutdown()
    {
        if(!this.isInitialized)
        {
            moduleError("Cannot shutdown Module. Module is not running (i.e., start was never called).");
            return false;
        }

        if(this.isTerminating)
        {
            moduleError("Failed to shutdown Module. Module.shutdown() was already called.");
        }
        
        this.isTerminating = true;

        this.runnableDispatcher.addRunnable(new FunctionRunnable(() -> terminateInternal()));


        while(this.isTerminating)
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
        
        this.runnableDispatcher.stop();
        return true;
    }

  
    private void initializeInternal(Properties properties)
    {
        this.initialize(properties);
        this.isInitialized = true;

        if(this.eventTracker != null)
        {
            this.eventTracker.onModuleStarted(this.id, this.type);
        }
    }

    protected abstract void initialize(Properties properties);

    private void terminateInternal()
    {
        this.unregisterAllPeriodicFunctions();
        this.terminate();
        this.isTerminating = false;
        if(this.eventTracker != null)
        {
            this.eventTracker.onModuleStopped(this.id, this.type);
        }
    }

    protected void terminate()
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

    public void setType(String type)
    {
        this.type = type;
    }

    public String getType()
    {
        return this.type;
    }

    public void setEventTracker(EventTracker tracker)
    {
        this.eventTracker = tracker;
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
            this.moduleFatal("Error in registerPeriodicFunction: Cannot register periodic function \"" + name + "\" with a period of 0 milliseconds.");
        }

        if (timers.containsKey(name))
        {
            this.moduleFatal("Tried to register function with name \"" + name + "\", but a periodic function with the same name was already registered before.");
        }

        FunctionRunnable runnable = new FunctionRunnable(callback, new ScheduleRepeatedIntervall(startTime, intervall));

       
        timers.put(name, runnable);
        this.runnableDispatcher.addRunnable(runnable);
    }

    protected void registerPeriodicFunction(final String name, Runnable callback, Duration intervall, LocalDateTime startTime, ScheduleTimeWindow onlyActiveBetween)
    {
        
        if (intervall.toMillis() == 0) 
        {
            this.moduleFatal("Error in registerPeriodicFunction: Cannot register periodic function \"" + name + "\" with a period of 0 milliseconds.");
        }

        if (timers.containsKey(name))
        {
            this.moduleFatal("Tried to register function with name \"" + name + "\", but a periodic function with the same name was already registered before.");
        }

        FunctionRunnable runnable = new FunctionRunnable(callback, new ScheduleRepeatedIntervall(startTime, intervall, onlyActiveBetween));

       
        timers.put(name, runnable);
        this.runnableDispatcher.addRunnable(runnable);
    }


    protected void unregisterPeriodicFunction(final String name)
    {
        if(!timers.containsKey(name))
        {
            moduleError("Error, tried to unregister periodic function but function was not found in list of registered timers."
            + "Was a function with this name ever registered before?");
            return;
        }
        
        ScheduledRunnable runnable = timers.get(name);
        if(runnable != null)
        {
            runnable.invalidate();
            runnableDispatcher.removeRunnable(runnable);
        }

        timers.remove(name);
    }

    protected void registerScheduledFunction(final String name, LocalDateTime dateTime, Runnable function)
    {   
        if(dateTime.isBefore(LocalDateTime.now()))
        {
            moduleWarning("Failed to schedule function \"" + name + "\" at time " + dateTime + ". The time is in the past. It is now: " + LocalDateTime.now());
        }

        FunctionRunnable runnable = new FunctionRunnable(function, new ScheduleOnce(dateTime));

        if(timers.containsKey((name)))
        {
            // Delete existing scheduled function with same name.
            timers.get(name).invalidate();
        }

        timers.put(name, runnable);
        this.runnableDispatcher.addRunnable(runnable);
    }

    private void unregisterAllPeriodicFunctions()
    {
        for(Map.Entry<String, ScheduledRunnable> entry : this.timers.entrySet())
        {
            entry.getValue().invalidate();
        }
    }

    public List<String> registerFunctionBasedOnSchedule(
            String name, Schedule schedule, Runnable function) 
    {

        List<String> timerNames = new ArrayList<>();
        
        // Counter and timestamp are used to provide a unique name for the registered function
        long timestamp = System.currentTimeMillis(); // get current timestamp in milliseconds
        
        for (SchedulePeriodic schedulePeriodic : schedule.getPeriodicList()) {
            String functionName = name + "_" + timestamp + "_" + timers.size();
            timerNames.add(functionName);

            if (schedulePeriodic.getIntervalCase() == SchedulePeriodic.IntervalCase.INTERVAL_NOT_SET) {
                throw new IllegalArgumentException(String.format(
                        "Cannot register periodic function \"%s\" based on schedule; " +
                        "no interval (i.e., frequency OR period) specified for schedule.",
                        name));
            }

            Duration interval = ScheduleHelper.getIntervalDurationFromPeriodicSchedule(schedulePeriodic);

            LocalDateTime startTime = LocalDateTime.now();
            if (schedulePeriodic.hasFirstExecutionTimeOfDay()) {
                startTime = ScheduleHelper.calculateNextTimeOfDay(schedulePeriodic.getFirstExecutionTimeOfDay());
            }

            if (schedulePeriodic.hasOnlyActiveBetweenTimeFrame()) {
                registerPeriodicFunction(functionName, function, interval, startTime, schedulePeriodic.getOnlyActiveBetweenTimeFrame());
            } else {
                registerPeriodicFunction(functionName, function, interval, startTime);
            }
        }

        for (ScheduleExactTime scheduleExactTime : schedule.getTimedList()) {
            String functionName = name + "_" + timestamp + "_" + timers.size();
            timerNames.add(functionName);

            LocalDateTime startTime = LocalDateTime.now();
            startTime = ScheduleHelper.calculateNextTimeOfDay(scheduleExactTime.getTimeOfDay());

            if (scheduleExactTime.getRepeatEveryNDays() != 0) {
                // If we have to repeat the function, schedule it as periodic function
                registerPeriodicFunction(functionName, function, Duration.ofDays(1), startTime);
            } else {
                // Otherwise, schedule it exactly once
                registerScheduledFunction(functionName, startTime, function);
            }
        }

        return timerNames;
    }

    protected boolean isPeriodicOrScheduledFunctionRegistered(String functionName)
    {
        return timers.containsKey(functionName);
    }

    protected void onConnectedToRemoteServer()
    {

    }
  
    protected void onDisconnectedFromRemoteServer()
    {

    }

    protected boolean isConnectedToRemoteServer()
    {
        Future<Boolean> future = this.isConnectedToRemoteServerRemoteFunction.execute();
        Boolean result = future.await();
        if(!future.wasExecutedSuccessfully())
        {
            return false;
        }
        return result.booleanValue();
    }

    public void notifyConnectedToRemoteServer()
    {
        if(!this.isInitialized)
        {
            return;
        }
        this.runnableDispatcher.addRunnable(new FunctionRunnable(() -> onConnectedToRemoteServer()));
    }
        
    public void notifyDisconnectedFromRemoteServer()
    {
        if(!this.isInitialized)
        {
            return;
        }
        this.runnableDispatcher.addRunnable(new FunctionRunnable(() -> onDisconnectedFromRemoteServer()));
    }

    public void enqueueRPC(DataPackage rpcRequest)
    {

        // Some entity requested for us to execute a remote function.
        // Create a ConsumerRunnable and enqueue it to execute the remote function of the thread of the Module.
        ConsumerRunnable<DataPackage> consumerRunnable = 
            new ConsumerRunnable<DataPackage>(rpcRequest, (request) -> executeRPCRequest(request), ScheduleOnce.now());

        ScheduledRunnable runnable = (ScheduledRunnable) consumerRunnable;

        this.runnableDispatcher.addRunnable(runnable);

    }



    // 
    private void executeRPCRequest(DataPackage rpcRequest)
    {

        if(!rpcRequest.getTargetModule().equals(this.id))
        {
            moduleError("Failed to execute RPC request. RPC is targeted for Module \"" + rpcRequest.getTargetModule() +
                 "\", but we are Module \"" + this.id + "\".");
            return;
        }

        boolean result = this.remoteFunctionRunnableHandler.executeRemoteFunctionRunnable(rpcRequest);
        if(!result)
        {
            moduleError("Failed to execute rpcRequest");
            return;
        }

    }

    protected boolean registerRemoteFunction(String functionName, Class<?> returnType, Class<?>... parameters)
    {
        return this.remoteFunctionRunnableHandler.registerRunnable(this, functionName, returnType, parameters);
    }

    protected <T> RemoteFunction<T> mapRemoteFunctionOfModule(String moduleId, String functionName, Class<T> returnType, Class<?>... parameters)
    {
        if(moduleId.equals(this.id))
        {
            moduleFatal("Cannot map remote function. Module tried to map function \"" + functionName + "\" of itself, which is not allowed.");
            return null;
        }
        return this.remoteFunctionHandler.mapModuleFunction(moduleId, functionName, returnType, parameters);
    }

    protected <T> RemoteFunction<T> mapRemoteFunctionOfRuntime(Runtime runtime, String functionName, Class<T> returnType, Class<?>... parameters)
    {
        return this.remoteFunctionHandler.mapRuntimeFunction(runtime, functionName, returnType, parameters);
    }

    public void pauseModule() 
    {
        if (isPaused) 
        {
            // moduleWarning("Failed to pause Module. Module is already paused.");
            return;
        }
        moduleInfo("Pausing Module");
        this.runnableDispatcher.addRunnable(new FunctionRunnable(() -> pauseInternal()));

        while (!isPaused) {
            try {
                Thread.sleep(20);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        runnableDispatcher.stop();
    }

    public void resumeModule() {
        if(!isPaused)
        {
            return;
        }
        moduleInfo("Resuming Module");

        isPaused = false;
        runnableDispatcher.start();
        this.runnableDispatcher.addRunnable(new FunctionRunnable(() -> resumeInternal()));
    }

    protected void pauseInternal() {
        moduleWarning("Paused.");
        onPause();
        isPaused = true;
        if(this.eventTracker != null)
        {
            this.eventTracker.onModulePaused(this.id, this.type);
        }
    }

    protected void resumeInternal() 
    {
        moduleWarning("Resumed.");
        if(this.eventTracker != null)
        {
            this.eventTracker.onModuleResumed(this.id, this.type);
        }

        onResume();
    }

    public void adjustPowerProfile(PowerProfile powerProfile) 
    {
        ConsumerRunnable<PowerProfile> consumerRunnable = new ConsumerRunnable<PowerProfile>(powerProfile, (profile) -> onPowerProfileChanged(profile), ScheduleOnce.now());
        ScheduledRunnable runnable = (ScheduledRunnable) consumerRunnable;

        this.runnableDispatcher.addRunnable(runnable);
        eventTracker.onModulePowerProfileApplied(this.id, this.type, powerProfile);
    }


    protected void onPause() {
    }

    protected void onResume() {
    }

    protected void onPowerProfileChanged(PowerProfile powerProfile) {
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