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

import adamma.c4dhi.claid_sensor_data.BatterySample;
import adamma.c4dhi.claid_sensor_data.BatteryData;
import adamma.c4dhi.claid_sensor_data.BatteryState;

import adamma.c4dhi.claid.Module.ModuleAnnotator;

public class BatteryCollector extends Module
{
    private Channel<BatteryData> batteryDataChannel;

    public static void annotateModule(ModuleAnnotator annotator)
    {
        annotator.setModuleDescription("The BatteryCollector allows to extract the current battery level and "
        + "charging state (i.e., charging, not charging, charging wirelessly, ...) of the device.\n"
        + "The BatteryCollector currently features one mode: Periodically.\n");
        // + "Periodically: In the periodic mode, the current battery level and charging state is extracted periodically (period can be configured)."
        // + "OnChange: The battery level and charging state will be extracted whenever there was a change to the battery state (e.g., battery level decreased or device was plugged / unplugged).");
    
        // annotator.describeProperty("monitoringMode", "String: Allows to specify how the battery information shall be monitored. Two options are available: \n"
        //                                                 + "Periodically: The current battery level and charging state are extracted periodically. Period can be configured via the monitoringIntervall property.\n"
        //                                                 + "OnChange: Battery level and information will be extracted whenever an update occured (e.g., battery level changed or device was plugged / unplugged).");
        annotator.describeProperty("monitoringIntervall:", "Number: Period (in milliseconds) by which the battery level and information will be extracted (e.g. 200ms -> 5 times per second).");
        annotator.describePublishChannel("BatteryData", BatteryData.class, "Channel to which the extracted BatteryInformation will be posted to. Data type is BatteryData.");
    }


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

        BatteryData.Builder batteryData = BatteryData.newBuilder();

        BatterySample batterySample = BatteryIntentHelper.extractBatterySampleFromIntent(batteryStatus);
        batteryData.addSamples(batterySample);

        batteryDataChannel.post(batteryData.build());
    }




}
