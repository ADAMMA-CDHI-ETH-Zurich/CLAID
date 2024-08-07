package adamma.c4dhi.claid_platform_impl;

import static android.content.Context.POWER_SERVICE;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.PowerManager;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;

public class GlobalDeviceSchedulerWakeUpReceiver extends BroadcastReceiver
{
    private PowerManager.WakeLock temporaryWakeLock = null;
    // Keep the device alive for at least 15 seconds.
    private final int TEMPORARY_WAKELOCK_TIMEOUT_MS = 15000;
    @Override
    public void onReceive(Context context, Intent intent) {
        writeToLogFile("GlobalDeviceSchedulerWakeupReceiver woke up.");
        // The device woke up.
        // We do nothing specific here, as we trust the individual RunnableDispatchers in each Runtime
        // to wake up, perform their tasks and reschedule the next device wakeup using the GlobalDeviceScheduler
        // of the middleware.
        // For safety, we will acquire a temporary wake lock just to give everyone enough time to wake up.
        if(temporaryWakeLock == null)
        {
            PowerManager powerManager = (PowerManager) context.getSystemService(POWER_SERVICE);
            temporaryWakeLock = powerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK,
                    "CLAID::GlobalDeviceSchedulerAndroid::PartialWakeLock");
        }

        // Double check for safety.
        if(temporaryWakeLock == null)
        {
            return;
        }

        if(temporaryWakeLock.isHeld())
        {
            return;
        }
        writeToLogFile("GlobalDeviceSchedulerWakeupReceiver acquiring temporary wake lock.");


        temporaryWakeLock.acquire(TEMPORARY_WAKELOCK_TIMEOUT_MS);
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
                DateTimeFormatter fileNameFormatter = DateTimeFormatter.ofPattern("dd-MM-yyyy");
                String fileNameFormattedDateTime = now.format(fileNameFormatter);
                // Open the file in append mode
                String filename = CLAID.getCommonDataPath() + "/wakeup_receiver_log-"  + fileNameFormattedDateTime + ".txt";
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
            logWriter.flush();

        } catch (IOException e) {
            e.printStackTrace();
        } 
    }
}