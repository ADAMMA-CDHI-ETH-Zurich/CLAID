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

import android.content.Intent;
import android.os.BatteryManager;

import adamma.c4dhi.claid_sensor_data.BatterySample;
import adamma.c4dhi.claid_sensor_data.BatteryState;


public class BatteryIntentHelper
{
    static BatterySample extractBatterySampleFromIntent(Intent batteryIntent)
    {
        // Are we charging / charged?
        int status = batteryIntent.getIntExtra(BatteryManager.EXTRA_STATUS, -1);
        boolean isCharging = status == BatteryManager.BATTERY_STATUS_CHARGING ||
                status == BatteryManager.BATTERY_STATUS_FULL;

        // How are we charging?
        int chargePlug = batteryIntent.getIntExtra(BatteryManager.EXTRA_PLUGGED, -1);
        boolean usbCharge = chargePlug == BatteryManager.BATTERY_PLUGGED_USB;
        boolean acCharge = chargePlug == BatteryManager.BATTERY_PLUGGED_AC;
        boolean wirelessCharge = chargePlug == BatteryManager.BATTERY_PLUGGED_WIRELESS;

        BatterySample.Builder batterySample = BatterySample.newBuilder();
        batterySample.setLevel(getBatteryLevel(batteryIntent));

        if (usbCharge)
        {
            batterySample.setState(BatteryState.USB_CHARGING);
        }
        else if (acCharge)
        {
            batterySample.setState(BatteryState.AC_CHARGING);
        }
        else if(wirelessCharge)
        {
            batterySample.setState(BatteryState.AC_CHARGING);
        }
        else if (isCharging)
        {
            batterySample.setState(BatteryState.WIRELESS_CHARGING);
        }
        else if (getBatteryLevel(batteryIntent) == 100)
        {
            batterySample.setState(BatteryState.FULL);
        }
        else
        {
            batterySample.setState(BatteryState.UNPLUGGED);
        }

        batterySample.setUnixTimestampInMs(System.currentTimeMillis());

        return batterySample.build();
    }

    static short getBatteryLevel(Intent batteryStatus)
    {
        int level = batteryStatus.getIntExtra(BatteryManager.EXTRA_LEVEL, -1);
        int scale = batteryStatus.getIntExtra(BatteryManager.EXTRA_SCALE, -1);
        short batteryLevel = (short) (level * 100 / (float)scale);
        return batteryLevel;
    }
}
