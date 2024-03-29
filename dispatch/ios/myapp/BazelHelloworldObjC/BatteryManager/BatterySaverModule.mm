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

#include "BatterySaverModule.hh"

#include "dispatch/core/Logger/Logger.hh"
#include "dispatch/core/proto_util.hh"
#include "dispatch/core/Utilities/StringUtils.hh"
namespace claid
{
    BatterySaverModule::BatterySaverModule() 
    {

    }

    BatterySaverModule::~BatterySaverModule() 
    {
   
    }

    void BatterySaverModule::initialize(Properties properties)
    {
        PowerSavingStrategyList strategies;
        properties.getObjectProperty("powerSavingStrategies", strategies);

        if(properties.wasAnyPropertyUnknown())
        {
            moduleFatal(properties.getMissingPropertiesErrorString());
            return;
        }
        Logger::logInfo("Strategies: %s", messageToString(strategies).c_str());
        
        for(const PowerSavingStrategy& strategy: strategies.strategies())
        {
            this->powerSavingStrategies.push_back(strategy);
        }

        registerPeriodicFunction("BatteryLevelMonitoring", &BatterySaverModule::getCurrentBatteryLevel, this, Duration::seconds(5));
    }


    void BatterySaverModule::getCurrentBatteryLevel() 
    {
        UIDevice* device = [UIDevice currentDevice];
        [device setBatteryMonitoringEnabled:YES];

        float level = (float)[device batteryLevel] * 100;

        if(level != this->lastBatteryLevel)
        {
            this->lastBatteryLevel = level;
            this->onBatteryLevelChanged(level);
        }
    }

    void BatterySaverModule::onBatteryLevelChanged(float level)
    {
        if(this->powerSavingStrategies.size() == 1)
        {
            if(level <= this->powerSavingStrategies[0].battery_threshold())
            {
                executeStrategy(0);
            }
            return;
        }
        // powerSavingStrategies are sorted by their battery_threshold in descending order.
        // (highest threshold is first in the list).
        for(int i = 0; i < this->powerSavingStrategies.size() - 1; i++)
        {
            // Find the strategy that we are currently in, which is the first strategy in the sorted list,
            // whose battery_threshold the current battery level beats. 
            if(level <= this->powerSavingStrategies[i].battery_threshold() && level >= this->powerSavingStrategies[i + 1].battery_threshold())
            {
                this->executeStrategy(i);
                return;
            }
        }

        if(level <= powerSavingStrategies[powerSavingStrategies.size() - 1].battery_threshold())
        {
            executeStrategy(powerSavingStrategies.size() - 1);
        }
    }

    void BatterySaverModule::executeStrategy(int strategyID)
    {
        if(strategyID == this->currentlyActiveStrategy)
        {
            return;
        }

        this->currentlyActiveStrategy = strategyID;

        const PowerSavingStrategy& strategy = this->powerSavingStrategies[strategyID];
        std::string strategyString = messageToString(strategy);
        StringUtils::stringReplaceAll(strategyString, "\n", "");

        Logger::logWarning("Battery level %f is below threshold %f, executing battery persevation strategy %s.", 
            this->lastBatteryLevel, strategy.battery_threshold(), strategyString.c_str());
        

        for(const std::string& activeModule : strategy.active_modules())
        {
            resumeModuleById(activeModule);
        }

        for(const std::string& pausedModule : strategy.paused_modules())
        {
            pauseModuleById(pausedModule);
        }
        
        // map<string, PowerProfile>, with string being the ModuleId.
        for(const auto& powerProfileForModule : strategy.power_profiles())
        {
            adjustPowerProfileOnModuleById(powerProfileForModule.first, powerProfileForModule.second);
        }

    }
}
REGISTER_MODULE(BatterySaverModule, claid::BatterySaverModule)