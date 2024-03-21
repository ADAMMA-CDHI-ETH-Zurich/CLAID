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

        registerPeriodicFunction("BatteryLevelMonitoring", &BatterySaverModule::getCurrentBatteryLevel, this, Duration::seconds(60));
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

        Logger::logWarning("Battery level %f is below threshold %f, \n",
            " executing battery persevation strategy %s.", this->lastBatteryLevel, strategy.battery_threshold(), strategyString.c_str());
        

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