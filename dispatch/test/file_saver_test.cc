/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Stephan Altmüller
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

#include "dispatch/core/Module/TypeMapping/TypeMapping.hh"
#include "gtest/gtest.h"

#include "dispatch/core/DataCollection/DataSaver/FileSaver.hh"
#include "dispatch/proto/sensor_data_types.pb.h"

using namespace claid;



std::shared_ptr<AccelerationData> getAccelerationData(int seed = 1)
{
    AccelerationData accelerationData;

    // Hardcode 10 AccelerationSamples
    for (int i = 0; i < 10; ++i) {
        AccelerationSample accelerationSample;
        accelerationSample.set_acceleration_x(seed * 1.0 * i);  // Replace with your hardcoded value
        accelerationSample.set_acceleration_y(seed * 2.0 * i);  // Replace with your hardcoded value
        accelerationSample.set_acceleration_z(seed * 3.0 * i);  // Replace with your hardcoded value
        accelerationSample.set_sensor_body_location("left_wrist");
        accelerationSample.set_unix_timestamp_in_ms(static_cast<uint64_t>(std::time(0)));  // Use a proper timestamp

        accelerationData.add_samples()->CopyFrom(accelerationSample);
    }

    return std::make_shared<AccelerationData>(accelerationData);

}

// Creates two simple Modules (Sender, Receiver) and registers them with the C++ Runtime.
// Starts the C++ runtime and and waits for the Receiver to receive data from the Sender.
TEST(FileSaverTestSuite, FileSaverTest)  
{

    const char* config_file = "dispatch/test/cpp_runtime_test.json";
    const std::string what = "test_data";
    const std::string storagePath = "data_test";
    const std::string fileNameFormat = "test.json";
    const std::string dataType = "json";


    FileSaver fileSaver;
    absl::Status status = fileSaver.initialize(what, storagePath, fileNameFormat, dataType, false);
    ASSERT_TRUE(status.ok()) << status;


    auto accelerationData = getAccelerationData(1);
    auto accelerationData2 = getAccelerationData(2);

   
    fileSaver.onNewData(std::static_pointer_cast<google::protobuf::Message>(accelerationData), Time::now());
    fileSaver.onNewData(std::static_pointer_cast<google::protobuf::Message>(accelerationData2), Time::now());

    status = fileSaver.endFileSaving();

    ASSERT_TRUE(status.ok()) << status;

}
