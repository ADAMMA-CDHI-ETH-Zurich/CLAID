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

#include "dispatch/core/CLAID.hh"
using namespace claid;


class SenderModule : public Module
{
    Channel<std::string> channel;

    int ctr = 0;

    void initialize(const std::map<std::string, std::string>& properties)
    {
        Logger::logInfo("SenderModule init!");

        int dataPeriodMs = 100;
        auto it = properties.find("dataPeriodMs");
        if(it != properties.end())
        {
            dataPeriodMs = std::atoi(it->second.c_str());
        }
        Logger::logInfo("Data period set to %d", dataPeriodMs);
        registerPeriodicFunction("TestFunction", &SenderModule::periodicFunction, this, Duration::milliseconds(dataPeriodMs));
        
        channel = publish<std::string>("StringData");    
    }

    void periodicFunction()
    {
        std::string string = "Test " + std::to_string(ctr);
        ctr++; 

        channel.post(string);
    }
};

class ReceiverModule : public Module
{
    Channel<std::string> channel;

    void initialize(const std::map<std::string, std::string>& properties)
    {
        channel = subscribe<std::string>("InputData", &ReceiverModule::onData, this);
    }

    void onData(ChannelData<std::string> data)
    {
        Logger::logInfo("Received data %s\n", data.getData().c_str());
    }
};  

REGISTER_MODULE_FACTORY_CUSTOM_NAME(TestSenderModule, SenderModule)
REGISTER_MODULE_FACTORY_CUSTOM_NAME(TestReceiverModule, ReceiverModule)

// Creates two simple Modules (Sender, Receiver) and registers them with the C++ Runtime.
// Starts the C++ runtime and and waits for the Receiver to receive data from the Sender.
TEST(CppRuntimeTestSuite, CppRuntimeTest)  
{
    std::cout << "T1\n";
    const char* socket_path = "/tmp/cpp_runtime_test_grpc.sock";
    const char* config_file = "dispatch/test/cpp_runtime_test.json";
    const char* host_id = "alex_client";
    const char* user_id = "user42";
    const char* device_id = "something_else";
    std::cout << "T2\n";

    CLAID claid;
    bool result = claid.start(socket_path, config_file, host_id, user_id, device_id);
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));

    ASSERT_TRUE(result) << claid.getStartStatus();
    claid.shutdown();
    std::cout << "Waiting\n";

}
