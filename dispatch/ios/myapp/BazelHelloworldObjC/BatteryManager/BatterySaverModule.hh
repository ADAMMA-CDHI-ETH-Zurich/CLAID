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

#include "dispatch/core/Module/ManagerModule.hh"
#include "dispatch/proto/claidservice.grpc.pb.h"

#import "Foundation/Foundation.h"
#import <UIKit/UIKit.h>

using claidservice::PowerSavingStrategy;
using claidservice::PowerProfile;

using namespace claid;
using namespace claidservice;

namespace claid
{
    class BatterySaverModule : public ManagerModule
    {
        private:
            std::vector<PowerSavingStrategy> powerSavingStrategies;


            void onBatteryLevelChanged(float level);
            void getCurrentBatteryLevel();
            void executeStrategy(int index);

            float lastBatteryLevel = -1.0;

            int currentlyActiveStrategy = -1;



        public:
            void initialize(Properties properties);

            BatterySaverModule();
            ~BatterySaverModule();
    };  
}
