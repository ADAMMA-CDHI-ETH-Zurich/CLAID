/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
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

#pragma once
#include "dispatch/core/CLAID.hh"
#include "dispatch/proto/sensor_data_types.pb.h"

using namespace claidservice;

#import <CoreMotion/CoreMotion.h>


namespace claid
{
    class AccelerometerCollector : public claid::Module
    {

        private:
            Channel<AccelerationData> accelerometerDataChannel;
            CMMotionManager* motionManager;

            AccelerationData accelerationData;

            bool samplingIsRunning;

            PowerProfile currentPowerProfile;
        

            void gatherAccelerometerSample();
            void postIfEnoughData();
        
            void startSampling();
            void stopSampling();
            void restartSampling();

            void onPause() override final;
            void onResume() override final;
            void onPowerProfileChanged(PowerProfile profile) override final;

        public:

            void initialize(Properties properties);
            


       
        
    };
}