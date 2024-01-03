package adamma.c4dhi.claid_android.collectors.battery;

import android.content.Intent;
import android.os.BatteryManager;

import adamma.c4dhi.claid_sensor_data.BatteryData;
import adamma.c4dhi.claid_sensor_data.BatteryState;


public class BatteryIntentHelper
{
    static BatteryData extractBatteryDataFromIntent(Intent batteryIntent)
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

        BatteryData.Builder batteryData = BatteryData.newBuilder();
        batteryData.setLevel(getBatteryLevel(batteryIntent));

        if (usbCharge)
        {
            batteryData.setState(BatteryState.USB_CHARGING);
        }
        else if (acCharge)
        {
            batteryData.setState(BatteryState.AC_CHARGING);
        }
        else if(wirelessCharge)
        {
            batteryData.setState(BatteryState.AC_CHARGING);
        }
        else if (isCharging)
        {
            batteryData.setState(BatteryState.WIRELESS_CHARGING);
        }
        else if (getBatteryLevel(batteryIntent) == 100)
        {
            batteryData.setState(BatteryState.FULL);
        }
        else
        {
            batteryData.setState(BatteryState.UNPLUGGED);
        }
        return batteryData.build();
    }

    static short getBatteryLevel(Intent batteryStatus)
    {
        int level = batteryStatus.getIntExtra(BatteryManager.EXTRA_LEVEL, -1);
        int scale = batteryStatus.getIntExtra(BatteryManager.EXTRA_SCALE, -1);
        short batteryLevel = (short) (level * 100 / (float)scale);
        return batteryLevel;
    }
}
