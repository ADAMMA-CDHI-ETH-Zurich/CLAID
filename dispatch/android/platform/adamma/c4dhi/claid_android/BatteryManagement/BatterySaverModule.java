package adamma.c4dhi.claid_android.BatteryManagement;

import adamma.c4dhi.claid.Module.ManagerModule;
import java.time.Duration;
import java.util.ArrayList;
import java.util.Map;

import android.os.BatteryManager;
import adamma.c4dhi.claid.Module.Channel;
import adamma.c4dhi.claid.Module.ChannelData;
import adamma.c4dhi.claid.Module.Module;
import adamma.c4dhi.claid_android.collectors.battery.BatteryIntentHelper;
import adamma.c4dhi.claid_platform_impl.CLAID;
import adamma.c4dhi.claid.PowerSavingStrategy;
import adamma.c4dhi.claid.PowerSavingStrategyList;
import adamma.c4dhi.claid.PowerProfile;
import adamma.c4dhi.claid.*;
import adamma.c4dhi.claid.Logger.Logger;

import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import java.util.Map;


public class BatterySaverModule extends ManagerModule
{
    
    private float lastBatteryLevel = -1;
    int currentlyActiveStrategy = -1;

    ArrayList<PowerSavingStrategy> powerSavingStrategies = new ArrayList<>();

    @Override
    public void initialize(Map<String, String> properties)
    {
        registerPeriodicFunction("BatteryLevelMonitoring", () -> getCurrentBatteryLevel(), Duration.ofSeconds(60));
        
    }

    public void getCurrentBatteryLevel() 
    {
        IntentFilter intentFilter = new IntentFilter(Intent.ACTION_BATTERY_CHANGED);
        Context context = CLAID.getContext();
        Intent batteryStatus = context.registerReceiver(null, intentFilter);

        float batteryLevel = getBatteryLevelFromIntent(batteryStatus);

        if (batteryLevel != lastBatteryLevel) 
        {
            lastBatteryLevel = batteryLevel;
            onBatteryLevelChanged(batteryLevel);
        }
    }

    static float getBatteryLevelFromIntent(Intent batteryStatus)
    {
        int level = batteryStatus.getIntExtra(BatteryManager.EXTRA_LEVEL, -1);
        int scale = batteryStatus.getIntExtra(BatteryManager.EXTRA_SCALE, -1);
        float batteryLevel = (float) (level * 100 / (float)scale);
        return batteryLevel;
    }

    public void onBatteryLevelChanged(float level) 
    {
        if (powerSavingStrategies.size() == 1) 
        {
            if (level <= powerSavingStrategies.get(0).getBatteryThreshold()) 
            {
                executeStrategy(0);
            }
            return;
        }

        for (int i = 0; i < powerSavingStrategies.size() - 1; i++) 
        {
            if (level <= powerSavingStrategies.get(i).getBatteryThreshold() &&
                level >= powerSavingStrategies.get(i + 1).getBatteryThreshold()) 
            {
                executeStrategy(i);
                return;
            }
        }
    }

    public void executeStrategy(int strategyID) 
    {
        if (strategyID == currentlyActiveStrategy) {
            return;
        }

        currentlyActiveStrategy = strategyID;

        PowerSavingStrategy strategy = powerSavingStrategies.get(strategyID);

        // Log strategy execution
        Logger.logWarning(String.format("Battery level %.2f is below threshold %.2f, executing battery preservation strategy.",
                lastBatteryLevel, strategy.getBatteryThreshold()));

        for (String activeModule : strategy.getActiveModulesList()) 
        {
            resumeModuleById(activeModule);
        }

        for (String pausedModule : strategy.getPausedModulesList()) 
        {
            pauseModuleById(pausedModule);
        }

        for (Map.Entry<String, PowerProfile> entry : strategy.getPowerProfiles().entrySet()) 
        {
            adjustPowerProfileOnModuleById(entry.getKey(), entry.getValue());
        }
    }

}
