/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Francesco Feher
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

#import "BatteryCollector.hh"

#import "Foundation/Foundation.h"
#import <UIKit/UIKit.h>
//TODO: test on real device, emulator produces wrong values

namespace claid
{
    void BatteryCollector::annotateModule(ModuleAnnotator& annotator)
    {
        annotator.setModuleCategory("DataCollection");
        annotator.setModuleDescription(absl::StrCat("The BatteryCollector allows to extract the current battery level and ",
        "charging state (i.e., charging, not charging, charging wirelessly, ...) of the device.\n",
        "The BatteryCollector currently features one mode: Periodically.\n"));
        // + "Periodically: In the periodic mode, the current battery level and charging state is extracted periodically (period can be configured)."
        // + "OnChange: The battery level and charging state will be extracted whenever there was a change to the battery state (e.g., battery level decreased or device was plugged / unplugged).");
    
        // annotator.describeProperty("monitoringMode", "String: Allows to specify how the battery information shall be monitored. Two options are available: \n"
        //                                                 + "Periodically: The current battery level and charging state are extracted periodically. Period can be configured via the monitoringIntervall property.\n"
        //                                                 + "OnChange: Battery level and information will be extracted whenever an update occured (e.g., battery level changed or device was plugged / unplugged).");
        annotator.describeProperty("monitoringIntervall", "Number: Period (in milliseconds) by which the battery level and information will be extracted (e.g. 200ms -> 5 times per second).", annotator.makeIntegerProperty(1, 200));
        annotator.describePublishChannel<BatteryData>("BatteryData", "Channel to which the extracted BatteryInformation will be posted to. Data type is BatteryData.");
    }

    void BatteryCollector::initialize(const std::map<std::string, std::string>& propertiesMap)
    {
        Logger::logInfo("Initializing");
        PropertyHelper properties(propertiesMap);

        properties.getProperty("samplingFrequency", this->samplingFrequency);
       
        if(properties.wasAnyPropertyUnknown())
        {
            this->moduleFatal(properties.getMissingPropertiesErrorString());
            return;
        }

        float samplingPeriod = 1000 / (1.0 * this->samplingFrequency);

        batteryDataChannel = publish<BatteryData>("BatteryData");
        registerPeriodicFunction("PostBatteryData", &BatteryCollector::sampleBatteryData, this, Duration::milliseconds(samplingPeriod));
    }
    
    void BatteryCollector::sampleBatteryData()
    {
        UIDevice *device = [UIDevice currentDevice];
        [device setBatteryMonitoringEnabled:YES];
        
        // 0 unknown, 1 unplegged, 2 charging, 3 full
        int state = (int)[device batteryState];
        int level = (int)[device batteryLevel] * 100;
        
        BatterySample& batterySample = *batteryData.add_samples();
        batterySample.set_level(level);
        switch (state) {
            case 1:
                batterySample.set_state(BatteryState::UNPLUGGED);
                break;
            case 2:
                batterySample.set_state(BatteryState::CHARGING);
                break;
            case 3:
                batterySample.set_state(BatteryState::FULL);
                break;
            default:
                batterySample.set_state(BatteryState::UNKNOWN);
                break;
        }

        postIfEnoughData();
    }


    void BatteryCollector::postIfEnoughData()
    {
        if(batteryData.samples().size() >= this->samplingFrequency)
        {
            batteryDataChannel.post(batteryData);
            batteryData.Clear();
        }
    }
}
REGISTER_MODULE(BatteryCollector, claid::BatteryCollector);