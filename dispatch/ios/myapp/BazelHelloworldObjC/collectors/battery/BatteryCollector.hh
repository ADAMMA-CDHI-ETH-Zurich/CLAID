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

#include "dispatch/core/CLAID.hh"
#include "dispatch/proto/sensor_data_types.pb.h"

// IMPORTAT: Test this only on real device, emulator produces wrong values

namespace claid
{
    class BatteryCollector : public claid::Module
    {
        
        private:
            Channel<BatteryData> batteryDataChannel;

            uint16_t samplingFrequency;

            BatteryData batteryData;
            void initialize(const std::map<std::string, std::string>& properties);
            void sampleBatteryData();
            void postIfEnoughData();

        public:

            static void annotateModule(ModuleAnnotator& annotator);
       
          
    };
}
