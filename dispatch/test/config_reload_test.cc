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

// Tests the reload functionality of the C++ Runtime.
// Starts CLAID with a config, then stops all modules and reload another config.
#include "dispatch/core/capi.h"
#include "gtest/gtest.h"
#include "dispatch/core/CLAID.hh"

using namespace claid;

bool testModule1Started = false;
bool testModule2Started = false;
bool testModule3Started = false;
bool testModule4Started = false;

class TestModule1 : public Module
{
    void initialize(Properties properties)
    {
        std::string prop; 

        properties.getStringProperty("TestModule1Data", prop);
        if(prop == "42")
        {
            testModule1Started = true;
        }
        
    }
};

REGISTER_MODULE(TestModule1, TestModule1);

class TestModule2 : public Module
{
    void initialize(Properties properties)
    {
        std::string prop; 

        properties.getStringProperty("TestModule2Data", prop);
        if(prop == "1337")
        {
            testModule2Started = true;
        }
    }
};

REGISTER_MODULE(TestModule2, TestModule2);


class TestModule3 : public Module
{
    void initialize(Properties properties)
    {
        std::string prop;
        Logger::logInfo("TestModule 3 initialize");
        properties.getStringProperty("TestModule3Data", prop);

        if(prop == "420")
        {
            testModule3Started = true;
        }
    }
};

REGISTER_MODULE(TestModule3, TestModule3);


class TestModule4 : public Module
{
    void initialize(Properties properties)
    {
        std::string prop;
        properties.getStringProperty("TestModule4Data", prop);
   
        if(prop == "96")
        {
            testModule4Started = true;
        }
        
    }
};

REGISTER_MODULE(TestModule4, TestModule4);


// Creates two simple Modules (Sender, Receiver) and registers them with the C++ Runtime.
// Starts the C++ runtime and and waits for the Receiver to receive data from the Sender.
TEST(ConfigReloadTestTestSuite, ConfigReloadTest)  
{

    std::cout << "\n\n\n==== TEST START ====\n\n\n\n" << std::flush;
    const char* config_file = "dispatch/test/config_reload_test_1.json";
    const char* socket_path = "/tmp/config_reload_test.sock";
    const char* host_id = "test_client";
    const char* user_id = "user42";
    const char* device_id = "something_else";


    CLAID claid;
    bool result = claid.start(socket_path, config_file, host_id, user_id, device_id);
    ASSERT_TRUE(result);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    ASSERT_TRUE(testModule1Started) << "TestModule1 did not start as expected";
    ASSERT_TRUE(testModule2Started) << "TestModule2 did not start as expected";

    ASSERT_TRUE(claid.loadNewConfig("dispatch/test/config_reload_test_2.json").ok());
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    testModule1Started = false;
    testModule2Started = false;
    ASSERT_FALSE(testModule1Started) << "TestModule1 was started but should not have been.";
    ASSERT_FALSE(testModule2Started) << "TestModule2 was started but should not have been.";
    ASSERT_TRUE(testModule3Started) << "TestModule3 did not start as expected";
    ASSERT_TRUE(testModule4Started) << "TestModule4 did not start as expected";
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    claid.shutdown();
}   