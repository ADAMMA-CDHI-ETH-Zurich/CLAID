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
#include "dispatch/core/CLAID.hh"

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



class TestSenderModule : public Module
{
    int ctr = 0;
    Channel<AccelerationData> outputChannel;
    void initialize(const std::map<std::string, std::string>& properties)
    {
        this->outputChannel = this->publish<AccelerationData>("Data");
        this->registerPeriodicFunction("DataGenerator", &TestSenderModule::periodicFunction, this, Duration::milliseconds(100));
    }

    void periodicFunction()
    {
        Logger::logInfo("Posting data");
        auto data = getAccelerationData(ctr);
        this->outputChannel.post(*data);
        ctr++;
    }
};

REGISTER_MODULE(TestSenderModule, TestSenderModule);

// Creates two simple Modules (Sender, Receiver) and registers them with the C++ Runtime.
// Starts the C++ runtime and and waits for the Receiver to receive data from the Sender.
TEST(DataSaverTestSuite, DataSaverJsonTest)  
{

    const char* socket_path = "unix:///tmp/data_saver_json_test_grpc.sock";
    const char* config_file = "dispatch/test/data_saver_json_test.json";
    const char* host_id = "alex_client";
    const char* user_id = "user42";
    const char* device_id = "something_else";

    CLAID claid;
    bool result = claid.start(socket_path, config_file, host_id, user_id, device_id);
    ASSERT_TRUE(result);



    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    claid.shutdown();
    std::cout << "Waiting\n";

    std::ifstream file("data_saver_json_test/test_data.json");
    ASSERT_TRUE(file.is_open());

    // TODO: Verify file content.
}
