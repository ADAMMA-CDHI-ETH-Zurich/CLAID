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

package adamma.c4dhi.claid_android.BatteryManagement;

import adamma.c4dhi.claid.Module.ManagerModule;
import java.time.Duration;
import java.util.ArrayList;
import java.util.Map;

import android.os.BatteryManager;
import adamma.c4dhi.claid.Module.Channel;
import adamma.c4dhi.claid.Module.ChannelData;
import adamma.c4dhi.claid.Module.Properties;
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
import java.util.List;


public class BatterySaverModule extends ManagerModule
{
    
    private float lastBatteryLevel = -1;
    int currentlyActiveStrategy = -1;

    List<PowerSavingStrategy> powerSavingStrategies;

    @Override
    public void initialize(Properties properties)
    {
        PowerSavingStrategyList strategies = properties.getObjectProperty("powerSavingStrategies", PowerSavingStrategyList.class);

        if(properties.wasAnyPropertyUnknown())
        {
            this.moduleFatal(properties.getMissingPropertiesErrorString());
            return;
        }
        Logger.logInfo("Strategies: " + strategies.getStrategiesList().toString());
        this.powerSavingStrategies = strategies.getStrategiesList();


        registerPeriodicFunction("BatteryLevelMonitoring", () -> getCurrentBatteryLevel(), Duration.ofSeconds(10));
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
        Logger.logInfo("Battery level changed: " + level);
        Logger.logInfo("Battery level num strategies: " + powerSavingStrategies.size());
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
            Logger.logInfo("Battery level checking: " + level + powerSavingStrategies.get(i).getBatteryThreshold() + " " + powerSavingStrategies.get(i + 1).getBatteryThreshold());

            if (level <= powerSavingStrategies.get(i).getBatteryThreshold() &&
                level >= powerSavingStrategies.get(i + 1).getBatteryThreshold()) 
            {
                executeStrategy(i);
                return;
            }
        }

        if(level <= powerSavingStrategies.get(powerSavingStrategies.size() - 1).getBatteryThreshold())
        {
            executeStrategy(powerSavingStrategies.size() - 1);
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

        if(strategy.getWakeLock() == true)
        {
            Logger.logWarning("enabling keep app awake.");
            CLAID.enableKeepAppAwake(CLAID.getContext());
        }
        else
        {
            Logger.logWarning("BatterySaver disabling keep app awake.");
            CLAID.disableKeepAppAwake(CLAID.getContext());
        }

        if(strategy.getDisableNetworkConnections())
        {
            deactivateNetworkConnections();
        }
        else
        {
            activateNetworkConnections();
        }

        if(strategy.getDisableWifiAndBluetooth())
        {
            if(!CLAID.DeviceOwnerFeatures.isDeviceOwner(CLAID.getContext()))
            {
                moduleError("Cannot disable Wifi and bluetooth. App is no device owner.");
                return;
            }

            CLAID.DeviceOwnerFeatures.disableWifi(CLAID.getContext());
            CLAID.DeviceOwnerFeatures.disableBluetooth(CLAID.getContext());
        }
        else
        {
            if(!CLAID.DeviceOwnerFeatures.isDeviceOwner(CLAID.getContext()))
            {
                moduleError("Cannot disable Wifi and bluetooth. App is no device owner.");
                return;
            }

            CLAID.DeviceOwnerFeatures.enableWifi(CLAID.getContext());
            CLAID.DeviceOwnerFeatures.enableBluetooth(CLAID.getContext());
        }
    }

}
