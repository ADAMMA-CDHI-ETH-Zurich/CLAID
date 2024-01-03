package adamma.c4dhi.claid_android.collectors.battery;

import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.util.Log;

import java.time.Duration;
import java.util.Map;

import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.Module.Channel;
import adamma.c4dhi.claid.Module.ChannelData;
import adamma.c4dhi.claid.Module.Module;

import adamma.c4dhi.claid_platform_impl.CLAID;


import adamma.c4dhi.claid_sensor_data.BatteryData;
import adamma.c4dhi.claid_sensor_data.BatteryState;


public class BatteryCollector extends Module
{
    private Channel<BatteryData> batteryDataChannel;


    String outputChannel = "";

    @Override
    public void initialize(Map<String, String> properties) 
    {
        if(!properties.containsKey("monitoringIntervall"))
        {
            this.moduleFatal("Property \"monitoringIntervall\" was not specified in the configuration file.");
        }

        Integer monitoringIntervall = Integer.parseInt(properties.get("monitoringIntervall"));
        Logger.logInfo("BatteryCollector started, monitoringIntervall is: " + monitoringIntervall + "ms");

        this.batteryDataChannel = this.publish("BatteryData", BatteryData.class);

        this.registerPeriodicFunction("PeriodicBatteryMonitoring", () -> postBatteryData(), Duration.ofMillis(monitoringIntervall));
        System.out.println("BatteryCollector initialized");
    }

    public void postBatteryData()
    {
        IntentFilter intentFilter = new IntentFilter(Intent.ACTION_BATTERY_CHANGED);
        Context context = CLAID.getContext();
        Intent batteryStatus = context.registerReceiver(null, intentFilter);

        BatteryData batteryData = BatteryIntentHelper.extractBatteryDataFromIntent(batteryStatus);

        batteryDataChannel.post(batteryData);
    }




}
