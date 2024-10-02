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

import android.content.Intent;
import android.content.IntentFilter;
import android.os.BatteryManager;
import android.os.PowerManager;

import adamma.c4dhi.claid.RemoteFunction.RemoteFunctionRunnableHandler;

public class DeviceInfoGathererAndroid
{

    public DeviceInfoGathererAndroid(RemoteFunctionRunnableHandler runnableHandler)
    {
        runnableHandler.registerRunnable(this, "android_get_battery_level_percentage", Double.class);
        runnableHandler.registerRunnable(this, "android_is_device_charging", Boolean.class);
    }

    public Double android_get_battery_level_percentage()
    {
        IntentFilter intentFilter = new IntentFilter(Intent.ACTION_BATTERY_CHANGED);
        Intent batteryStatus = CLAID.getContext().registerReceiver(null, intentFilter);

        if (batteryStatus != null) {
            int level = batteryStatus.getIntExtra(BatteryManager.EXTRA_LEVEL, -1);
            int scale = batteryStatus.getIntExtra(BatteryManager.EXTRA_SCALE, -1);

            // Calculate battery percentage
            double batteryPercentage = (level / (double) scale) * 100;
            return batteryPercentage;
        } else {
            return -1.0;
        }
    }

    public Boolean android_is_device_charging()
    {
        IntentFilter intentFilter = new IntentFilter(Intent.ACTION_BATTERY_CHANGED);
        Intent batteryStatus = CLAID.getContext().registerReceiver(null, intentFilter);

        if (batteryStatus != null) {
            int status = batteryStatus.getIntExtra(BatteryManager.EXTRA_STATUS, -1);
            return status == BatteryManager.BATTERY_STATUS_CHARGING;
        } else {
            return false; // Assume not charging if we can't retrieve status
        }
    }

}
