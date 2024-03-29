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

import adamma.c4dhi.claid_sensor_data.BatterySample;
import adamma.c4dhi.claid_sensor_data.BatteryData;
import adamma.c4dhi.claid_sensor_data.BatteryState;


import static android.app.AlarmManager.RTC_WAKEUP;
import static android.app.PendingIntent.FLAG_UPDATE_CURRENT;
import static android.app.PendingIntent.FLAG_MUTABLE;

import static android.app.PendingIntent.getBroadcast;
import static android.content.Context.ALARM_SERVICE;
import static android.app.AlarmManager.RTC_WAKEUP;
import static android.app.PendingIntent.FLAG_UPDATE_CURRENT;
import static android.app.PendingIntent.getBroadcast;
import static android.content.Context.ALARM_SERVICE;


import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;


import java.util.Calendar;

public class AlarmBatteryCollector extends Module
{

    public void initialize(Properties properties)
    {
        moduleInfo("AlarmBatteryCollector init");
        Context context = CLAID.getContext();

        Calendar calendar = Calendar.getInstance();
        calendar.setTimeInMillis(System.currentTimeMillis()+ 10000);
        //noinspection ConstantConditions


      //  startActivity(new Intent(android.provider.Settings.ACTION_REQUEST_SCHEDULE_EXACT_ALARM, Uri.parse("package:"+ getPackageName())));
        final AlarmManager am = (AlarmManager) context.getSystemService(ALARM_SERVICE);
        final Intent intent = new Intent(context, adamma.c4dhi.claid_android.collectors.battery.BatteryReceiver.class);
        final PendingIntent pending = getBroadcast(context, 0, intent, FLAG_UPDATE_CURRENT|FLAG_MUTABLE);


        am.setExactAndAllowWhileIdle(RTC_WAKEUP, calendar.getTimeInMillis(), pending);

    }

   

}
