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
package adamma.c4dhi.claid_platform_impl;

import static android.app.AlarmManager.RTC_WAKEUP;
import static android.app.PendingIntent.FLAG_MUTABLE;
import static android.app.PendingIntent.FLAG_UPDATE_CURRENT;
import static android.app.PendingIntent.getBroadcast;
import static android.content.Context.ALARM_SERVICE;
import static android.content.Context.POWER_SERVICE;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.Buffer;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.Calendar;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.os.PowerManager;

import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.RemoteFunction.RemoteFunctionRunnableHandler;

public class GlobalDeviceSchedulerAndroid
{
    private PowerManager.WakeLock wakeLock;

    public GlobalDeviceSchedulerAndroid(RemoteFunctionRunnableHandler runnableHandler)
    {
        runnableHandler.registerRunnable(this, "android_acquire_wakelock", Void.class);
        runnableHandler.registerRunnable(this, "android_release_wakelock", Void.class);
        runnableHandler.registerRunnable(this, "android_schedule_device_wakeup_at", Void.class, Long.class);
    }

    private boolean createWakeLocKIfNotExists()
    {
        Context context = CLAID.getContext();
        if(context == null)
        {
            writeToLogFile("Cannot create WakeLock, context is null");
            return false;
        }
        if(wakeLock != null)
        {
            return true;
        }
        PowerManager powerManager = (PowerManager) context.getSystemService(POWER_SERVICE);
        wakeLock = powerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK,
                "CLAID::GlobalDeviceSchedulerAndroid::PartialWakeLock");
        writeToLogFile("Created WakeLock");

        return true;
    }

    public void android_acquire_wakelock()
    {
        if(!createWakeLocKIfNotExists())
        {
            Logger.logError("Failed to acquire wake lock! Cannot create WakeLock object.");
            return;
        }
        writeToLogFile("android_acquire_wakelock called");

        if(!wakeLock.isHeld())
        {
            writeToLogFile("android_acquire_wakelock acquired");
            wakeLock.acquire();
        }
        else
        {
            writeToLogFile("android_acquire_wakelock do nothing because wake lock is already held");
        }
    }

    public void android_release_wakelock()
    {
        if(!createWakeLocKIfNotExists())
        {
            Logger.logError("Failed to acquire wake lock! Cannot create WakeLock object.");
            return;
        }
        writeToLogFile("android_release_wakelock called");
        if(wakeLock.isHeld())
        {
            writeToLogFile("android_release_wakelock releasing wakelock");
            wakeLock.release();
        }
    }

    public void android_schedule_device_wakeup_at(Long scheduledTimeUnixTimestamp)
    {
        writeToLogFile("android_schedule_device_wakeup_at called");

        Context context = CLAID.getContext();

        if(context == null)
        {
            Logger.logError("Failed to schedule device wake up at time. CLAID context is null.");
            return;
        }
 
        Calendar calendar = Calendar.getInstance();

        writeToLogFile("android_schedule_device_wakeup_at scheduling");

        long milliseconds = scheduledTimeUnixTimestamp.longValue();
        calendar.setTimeInMillis(milliseconds);

        final AlarmManager am = (AlarmManager) context.getSystemService(ALARM_SERVICE);
        final Intent intent = new Intent(context, adamma.c4dhi.claid_platform_impl.GlobalDeviceSchedulerWakeUpReceiver.class);
        final PendingIntent pending = getBroadcast(context, 0, intent, FLAG_UPDATE_CURRENT|FLAG_MUTABLE);

        Logger.logInfo("Scheduling device wakeup at: " + calendar.toString());
        writeToLogFile("Scheduling device wakeup at: " + calendar.toString());

        am.setExactAndAllowWhileIdle(RTC_WAKEUP, calendar.getTimeInMillis(), pending);
    }

    private BufferedWriter logWriter = null;
    private void writeToLogFile(String text) {
        try {
            // Get the current date and time
            LocalDateTime now = LocalDateTime.now();
            DateTimeFormatter formatter = DateTimeFormatter.ofPattern("dd-MM-yyyy_HH-mm-ss");
            String formattedDateTime = now.format(formatter);
            if(logWriter == null)
            {
                // Open the file in append mode
                String filename = CLAID.getCommonDataPath() + "/scheduler_log-"  + formattedDateTime + ".txt";
                Logger.logInfo("Opening scheduler log " + filename);
                logWriter = new BufferedWriter(new FileWriter(filename, true));
            }

            // Get the Unix timestamp
            long unixTimestamp = System.currentTimeMillis();

            // Prepend the date, time, and Unix timestamp to the text
            String logEntry = formattedDateTime + " [" + unixTimestamp + "] " + text;

            // Write the text to the file
            logWriter.write(logEntry);
            // New line for better readability in the log file
            logWriter.newLine();
        } catch (IOException e) {
            e.printStackTrace();
        } 
    }
}
