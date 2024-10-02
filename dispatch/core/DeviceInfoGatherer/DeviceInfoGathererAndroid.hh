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
#pragma once
#include <map>
#include "dispatch/proto/claidservice.grpc.pb.h"
#include "dispatch/core/Utilities/Time.hh"
#include "dispatch/core/module_table.hh"
#include "dispatch/core/RemoteFunction/RemoteFunctionHandler.hh"

namespace claid {

class DeviceInfoGatherer
{
private:
    RemoteFunction<double> androidGetDeviceBatteryLevelPercentage;
    RemoteFunction<bool> androidIsDeviceCharging;

public:
 
    DeviceInfoGatherer(RemoteFunctionHandler remoteFunctionHandler) : remoteFunctionHandler(remoteFunctionHandler)
    {
        androidGetDeviceBatteryLevelPercentage = remoteFunctionHandler.mapRuntimeFunction<void>(Runtime::RUNTIME_JAVA, "android_get_battery_level_percentage");
        androidIsDeviceCharging = remoteFunctionHandler.mapRuntimeFunction<void>(Runtime::RUNTIME_JAVA, "android_is_device_charging");
    }

    virtual bool isAvailable()
    {
        return true;
    }

    virtual double getDeviceBatteryLevelPercentage()
    {
        auto future = androidGetDeviceBatteryLevelPercentage();
        double percentage = future->await();

        if(!future->wasExecutedSuccessfully())
        {
            return -1;
        }

        return percentage;
    }

    virtual DeviceChargingState getDeviceChargingState()
    {
        auto future = androidIsDeviceCharging();
        bool charging = future->await();

        if(!future->wasExecutedSuccessfully())
        {
            return DeviceChargingState::UNKNOWN;
        }
        else if(charging)
        {
            return DeviceChargingState::CHARGING;
        }
        else
        {
            return DeviceChargingState::NOT_CHARGING;
        }
    }
};

}