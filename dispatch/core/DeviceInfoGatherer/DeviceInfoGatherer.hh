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
#include "dispatch/proto/claidservice.pb.h"
#include "dispatch/core/Utilities/Time.hh"
#include "dispatch/core/module_table.hh"
#include "dispatch/core/RemoteFunction/RemoteFunctionHandler.hh"
#include "dispatch/core/DeviceInfoGatherer/DeviceChargingState.hh"

namespace claid {

class DeviceInfoGatherer
{
    private:
        RemoteFunctionHandler& remoteFunctionHandler;
    
    public:

        DeviceInfoGatherer(RemoteFunctionHandler& remoteFunctionHandler) : remoteFunctionHandler(remoteFunctionHandler)
        {

        }

        virtual ~DeviceInfoGatherer() {}

        virtual bool isAvailable()
        {
            return false;
        }

        virtual double getDeviceBatteryLevelPercentage()
        {
            return -1;
        }

        virtual DeviceChargingState getDeviceChargingState()
        {
            return DeviceChargingState::DEVICE_CHARGING_STATE_UNKNOWN;
        }
};

}