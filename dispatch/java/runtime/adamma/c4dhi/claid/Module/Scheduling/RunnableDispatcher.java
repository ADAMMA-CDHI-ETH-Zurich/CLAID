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

package adamma.c4dhi.claid.Module.Scheduling;

import java.time.temporal.ChronoUnit;
import java.time.temporal.Temporal;
import java.time.temporal.TemporalUnit;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid_platform_impl.CLAID;
import adamma.c4dhi.claid.RemoteFunction.RemoteFunctionHandler;
import adamma.c4dhi.claid.RemoteFunction.RemoteFunction;
import adamma.c4dhi.claid.RuntimeType;
import adamma.c4dhi.claid.Runtime;

import java.util.concurrent.TimeUnit;
import java.util.Map;
import java.util.TreeMap;
import java.util.List;
import java.sql.Time;
import java.time.LocalDateTime;
import java.time.ZoneId;
import java.util.ArrayList;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;


import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.time.Duration;
import java.time.ZoneOffset;
import java.time.Instant;
public class RunnableDispatcher 
{
    private Map<LocalDateTime, ScheduledRunnable> scheduledRunnables = new TreeMap<>();
    private final Lock mutex = new ReentrantLock();
    private final Condition conditionVariable = mutex.newCondition();
    private boolean rescheduleRequired = false;
    private boolean running = false;

    private Thread scheduleThread;

    // Allows the RunnableDispatcher to make calls into the Middleware, e.g.,
    // to schedule device wake ups or request to keep the device awake.
    private RemoteFunctionHandler remoteFunctionHandler;

    private RemoteFunction<Void> middlewareFuncAcquireWakeLock;
    private RemoteFunction<Void> middlewareFuncReleaseLock;
    private RemoteFunction<Void> middlewareFuncScheduleDeviceWakeUpAt;

    private long localDateTimeToMilliseconds(LocalDateTime localDateTime)
    {
        long milliseconds = localDateTime.atZone(ZoneId.systemDefault()).toInstant().toEpochMilli();
        return milliseconds;
    }

    private long getWaitDurationUntilNextRunnableIsDue() 
    {
        mutex.lock();
        try 
        {
            if (scheduledRunnables.isEmpty()) 
            {
                // Wait forever.
                return Long.MAX_VALUE;
            }

            LocalDateTime now = LocalDateTime.now();
            LocalDateTime nextExecutionTime = scheduledRunnables.keySet().iterator().next();

            long timeUntilNextRunnableIsDue = localDateTimeToMilliseconds(nextExecutionTime) - localDateTimeToMilliseconds(now);
            writeToLogFile("Calculating getWaitDurationUntilNextRunnableIsDue " + now.toString() + " " + nextExecutionTime.toString() + " " + timeUntilNextRunnableIsDue);

            // Make sure we do not return a negative duration.
            return timeUntilNextRunnableIsDue < 0 ? 0 : timeUntilNextRunnableIsDue;
        } 
        finally 
        {
            mutex.unlock();
        }
    }

    private void waitUntilRunnableIsDueOrRescheduleIsRequired() 
    {

        long waitTime = getWaitDurationUntilNextRunnableIsDue();



        mutex.lock();
        try {

             // Get the current LocalDateTime
            LocalDateTime now = LocalDateTime.now();

            // Define the offset in milliseconds
            long offsetMillis = waitTime; // Example: 1000 milliseconds (1 second)

            // Add the offset to the current time
            LocalDateTime newTime = now.plus(offsetMillis, ChronoUnit.MILLIS);
            // Define the formatter for the desired format
            DateTimeFormatter formatter = DateTimeFormatter.ofPattern("dd.MM.yyyy-HH:mm:ss");

            // Format the new time as a string
            String formattedTime = newTime.format(formatter);

            writeToLogFile("condition variable waiting for "  + waitTime + " milliseconds, waking up at " + formattedTime);



            middlewareScheduleDeviceWakeupAt(Long.valueOf(System.currentTimeMillis() + waitTime));

            conditionVariable.await(waitTime, TimeUnit.MILLISECONDS);

            writeToLogFile("Condition variable awoke");
        } catch (InterruptedException e) {
            writeToLogFile("Interrupted exception");
            Thread.currentThread().interrupt();

        } finally {
            writeToLogFile("Condition variable awoke unlocking");
            mutex.unlock();
        }
    }

    private void processRunnable(ScheduledRunnable scheduledRunnable) {
        if (!running) {
            return;
        }

        if (scheduledRunnable.isValid()) 
        {

            scheduledRunnable.run();
            


            /*if (scheduledRunnable.runnable.stopDispatcherAfterThisRunnable) {
                // No more runnables will be executed after this one!
                stop();
                System.out.println("STOPPED DISPATCHER!");
                return;
            }*/

            if (scheduledRunnable.schedule.doesRunnableHaveToBeRepeated())
             {
                writeToLogFile("Runnable needs to be repeated ");

                mutex.lock();
                try {

                    scheduledRunnable.schedule.updateExecutionTime();

                    LocalDateTime newExecutionTime = scheduledRunnable.schedule.getExecutionTime();
                    writeToLogFile("Runnable new execution time " + newExecutionTime.toString());


                    // Reinsert the runnable with the new scheduled execution time.
                    // Note that the runnable was removed from scheduledRunnables in the getAndRemoveDueRunnables() function.
                    scheduledRunnables.put(newExecutionTime, scheduledRunnable);
                } finally {
                    mutex.unlock();
                }
            }
        }
        else
        {
            Logger.logInfo("Runnable not valid.");
        }
    }

    private void processRunnables(List<ScheduledRunnable> runnables) {

        for (ScheduledRunnable runnable : runnables) {
            processRunnable(runnable);
        }
    }

    private List<ScheduledRunnable> getAndRemoveDueRunnables() {
        List<ScheduledRunnable> runnables = new ArrayList<>();
        long now = System.currentTimeMillis();

        mutex.lock();
        try {
            List<LocalDateTime> keysToRemove = new ArrayList<>();

            for (Map.Entry<LocalDateTime, ScheduledRunnable> entry : scheduledRunnables.entrySet()) {
                LocalDateTime dueTime = entry.getKey();

                long dueTimeMillis = localDateTimeToMilliseconds(dueTime);
                if (now >= dueTimeMillis) {
                    runnables.add(entry.getValue());
                    keysToRemove.add(dueTime);
                } else {
                    // Since the map is ordered in ascending order,
                    // all subsequent entries have a time
                    // which is greater than the time of the current entry.
                    // Hence, no need to keep iterating. We found the latest-most runnable
                    // which is due.
                    break;
                }
            }

            // If at least one runnable was added to runnables, that means we have to remove them from scheduledRunnables.
            for (LocalDateTime key : keysToRemove) {
                scheduledRunnables.remove(key);
            }
        } finally {
            mutex.unlock();
        }

        return runnables;
    }

    public void setRemoteFunctionHandler(RemoteFunctionHandler remoteFunctionHandler)
    {
        this.remoteFunctionHandler = remoteFunctionHandler;
        this.middlewareFuncAcquireWakeLock = 
            remoteFunctionHandler.mapRuntimeFunction(Runtime.MIDDLEWARE_CORE, "acquire_wakelock", Void.class, RuntimeType.class);

        this.middlewareFuncReleaseLock = 
            remoteFunctionHandler.mapRuntimeFunction(Runtime.MIDDLEWARE_CORE, "release_wakelock", Void.class, RuntimeType.class);

        this.middlewareFuncScheduleDeviceWakeUpAt = 
            remoteFunctionHandler.mapRuntimeFunction(Runtime.MIDDLEWARE_CORE, "schedule_device_wakeup_at", Void.class, RuntimeType.class, Long.class);
    }

    public boolean start() 
    {
        if(running)
        {
            return false;
        }
        running = true;
        scheduleThread = new Thread()
        {
            public void run()
            {
                runScheduling();
            }
        };
        scheduleThread.start();
        return true;
    }

    public boolean stop() {
        if(!running)
        {
            return false;
        }
        running = false;

        return true;
    }

    public void addRunnable(ScheduledRunnable runnable) {
        mutex.lock();
        try {
            LocalDateTime executionTime = runnable.schedule.getExecutionTime();
            
            // Ensure that we do not override an existing runnable.
            while(scheduledRunnables.containsKey(executionTime))
            {
                    Logger.logInfo("Runnables contains key");

                // Reschedule the runnable a bit later.
                // Plus 1000 ns = plus 1 ms
                executionTime = executionTime.plusNanos(1000);
            }

            scheduledRunnables.put(executionTime, runnable);
                Logger.logInfo("Runnables " + scheduledRunnables.size());
            // This will lead to a wake-up, so we can reschedule.
            rescheduleRequired = true;

            conditionVariable.signalAll();

        } finally {
            mutex.unlock();
        }

    }

    public void runScheduling() {
        List<ScheduledRunnable> dueRunnables = new ArrayList<>();
        while (running) {
            middlewareAcquireWakeLock();
            do {
                writeToLogFile("Scheduler woke up.");

                // While we process runnables, it is possible
                // that another runnable becomes due in the meantime.
                // Hence, we repeat this loop until there are no more
                // runnables that are due.
                dueRunnables = getAndRemoveDueRunnables();

                writeToLogFile("Scheduler due runnables: " + dueRunnables.size());

                processRunnables(dueRunnables);
            } while (!dueRunnables.isEmpty());
            rescheduleRequired = false;
            middlewareReleaseWakeLock();
            waitUntilRunnableIsDueOrRescheduleIsRequired();
        }
    }

    private FileOutputStream fos = null;
    void writeToLogFile(String data) 
    {
        Logger.logInfo(data);
        
        try {

            if(fos == null)
            {
                // Create a new File instance
                File file = new File(CLAID.getMediaDirPath(CLAID.getContext()) + "/java_scheduler_log.txt");

                // Use FileOutputStream to open the file in append mode
                fos = new FileOutputStream(file, true);
            }
            // Get the current date and time
            String dateTime = getCurrentDateTime();

            // Combine date, time, and data
            String entry = dateTime + " - " + data + "\n";

            // Write the data to the file
            fos.write(entry.getBytes());

            // Close the file output stream
        } catch (IOException e) {
            e.printStackTrace();
        } 
    }

    private static String getCurrentDateTime() {
        // Get the current date and time in the desired format
        SimpleDateFormat dateFormat = new SimpleDateFormat("dd-MM-yyyy HH:mm:ss", Locale.getDefault());
        Date currentDate = new Date();
        return dateFormat.format(currentDate);
    }

    RuntimeType getRuntimeType()
    {
        RuntimeType.Builder type = RuntimeType.newBuilder();
        type.setRuntime(Runtime.RUNTIME_JAVA);
        return type.build();
    }

    private void middlewareAcquireWakeLock()
    {
        if(remoteFunctionHandler == null)
        {
            return;
        }
        middlewareFuncAcquireWakeLock.execute(getRuntimeType());
    }

    private void middlewareReleaseWakeLock()
    {
        if(remoteFunctionHandler == null)
        {
            return;
        }
        middlewareFuncReleaseLock.execute(getRuntimeType());
    }

    private void middlewareScheduleDeviceWakeupAt(Long milliseconds)
    {
        if(remoteFunctionHandler == null)
        {
            return;
        }
        middlewareFuncScheduleDeviceWakeUpAt.execute(getRuntimeType(), milliseconds);
    }
}
