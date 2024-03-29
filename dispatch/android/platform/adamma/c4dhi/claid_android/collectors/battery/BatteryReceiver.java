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

package adamma.c4dhi.claid_android.collectors.battery;

import adamma.c4dhi.claid.Module.Module;
import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.Module.Channel;
import adamma.c4dhi.claid.Module.ChannelData;
import adamma.c4dhi.claid.Module.Module;
import adamma.c4dhi.claid.Module.Properties;

import adamma.c4dhi.claid_platform_impl.CLAID;
import static java.lang.String.format;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
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

import android.content.Intent;
import android.content.IntentFilter;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import android.os.BatteryManager;
import static android.app.AlarmManager.RTC_WAKEUP;
import static android.app.PendingIntent.FLAG_UPDATE_CURRENT;
import static android.app.PendingIntent.FLAG_MUTABLE;


import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;

import static android.app.PendingIntent.getBroadcast;
import static android.content.Context.ALARM_SERVICE;
import static android.app.AlarmManager.RTC_WAKEUP;
import static android.app.PendingIntent.FLAG_UPDATE_CURRENT;
import static android.app.PendingIntent.getBroadcast;
import static android.content.Context.ALARM_SERVICE;


public class BatteryReceiver extends BroadcastReceiver {


    @Override
    public void onReceive(Context context, Intent intent) {

        Logger.logInfo("BatteryReceiver on receive");
        IntentFilter intentFilter = new IntentFilter(Intent.ACTION_BATTERY_CHANGED);
        Intent batteryStatus = context.registerReceiver(null, intentFilter);

        writeToLogFile(System.currentTimeMillis() + "," + getBatteryLevel(batteryStatus));

        final Intent intentNew = new Intent(context, BatteryReceiver.class);
        final AlarmManager am = (AlarmManager) context.getSystemService(ALARM_SERVICE);

        final PendingIntent pending = getBroadcast(context, 0, intent, FLAG_UPDATE_CURRENT|FLAG_MUTABLE);
      

        am.setExactAndAllowWhileIdle(RTC_WAKEUP, System.currentTimeMillis() + 5*60*1000, pending);
    }

    static short getBatteryLevel(Intent batteryStatus)
    {
        int level = batteryStatus.getIntExtra(BatteryManager.EXTRA_LEVEL, -1);
        int scale = batteryStatus.getIntExtra(BatteryManager.EXTRA_SCALE, -1);
        short batteryLevel = (short) (level * 100 / (float)scale);
        return batteryLevel;
    }

    private FileOutputStream fos = null;

    void writeToLogFile(String data) 
    {
        
        try {

            if(fos == null)
            {
                // Create a new File instance
                File file = new File(CLAID.getMediaDirPath(CLAID.getContext()) + "/battery_exact.txt");

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
}
  