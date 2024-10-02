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

// Tests the API of modules, especially registering and unregistering functions

#include "dispatch/core/Module/TypeMapping/TypeMapping.hh"
#include "gtest/gtest.h"

#include "dispatch/core/CLAID.hh"
using namespace claid;

bool finished = false;

class TestModule : public Module
{
    void initialize(Properties properties)
    {
        ASSERT_EQ(timers.size(), 0) << "Invalid number of timers";
        registerScheduledFunction("BeginTest", Time::now(), &TestModule::beginTest, this);   
    }

    void beginTest()
    {
        ASSERT_EQ(timers.size(), 1) << "Invalid number of timers";
        unregisterPeriodicFunction("BeginTest");
        testRegisterUnregisterPeriodic();
        testRegisterUnregisterScheduled();
        testRegisterUnregisterBasedOnSchedule();
        registerScheduledFunction("EndTest", Time::now() + Duration::seconds(5), &TestModule::endTest, this);  
    }

    void testRegisterUnregisterPeriodic()
    {

    }

    void testRegisterUnregisterScheduled()
    {

    }

    void testRegisterUnregisterBasedOnSchedule()
    {

    }

    void endTest()
    {
        ASSERT_EQ(timers.size(), 1) << "Invalid number of timers";
        finished = true;
    }
};

REGISTER_MODULE_FACTORY_CUSTOM_NAME(TestModule, TestModule)


// Creates two simple Modules (Sender, Receiver) and registers them with the C++ Runtime.
// Starts the C++ runtime and and waits for the Receiver to receive data from the Sender.
TEST(ModuleTestSuite, ModuleTest)  
{
    const char* socket_path = "/tmp/module_test.sock";
    const char* config_file = "dispatch/test/module_test.json";
    const char* host_id = "test_client";
    const char* user_id = "user42";
    const char* device_id = "something_else";

    CLAID claid;
    bool result = claid.start(socket_path, config_file, host_id, user_id, device_id);
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));

    ASSERT_TRUE(result) << claid.getStartStatus();

    while(!finished)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    claid.shutdown();
}
