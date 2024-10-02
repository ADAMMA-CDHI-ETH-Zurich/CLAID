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
bool aborted = true;
class TestModule : public Module
{
    void initialize(Properties properties)
    {
        ASSERT_EQ(timers.size(), 0) << "Invalid number of timers";
        registerScheduledFunction("BeginTest", Time::now() + Duration::seconds(2), &TestModule::beginTest, this);   
    }

    void beginTest()
    {
        ASSERT_EQ(timers.size(), 1) << "Invalid number of timers";
        unregisterPeriodicFunction("BeginTest");
        testRegisterUnregisterPeriodic();
        testRegisterUnregisterScheduled();
        testRegisterUnregisterBasedOnSchedule();
        moduleInfo("registering end test");
        registerScheduledFunction("EndTest", Time::now() + Duration::seconds(5), &TestModule::endTest, this);  
    }

    void testRegisterUnregisterPeriodic()
    {
        registerPeriodicFunction("SomeFunction", &TestModule::someFunction, this, Duration::seconds(5));
        ASSERT_EQ(timers.size(), 1) << "Invalid number of timers";
        registerPeriodicFunction("SomeFunction2", &TestModule::someFunction, this, Duration::seconds(10));
        registerPeriodicFunction("SomeFunction3", &TestModule::someFunction, this, Duration::seconds(42));
        ASSERT_EQ(timers.size(), 3) << "Invalid number of timers";
        unregisterPeriodicFunction("SomeFunction");
        ASSERT_EQ(timers.size(), 2) << "Invalid number of timers";
        unregisterPeriodicFunction("SomeFunction2");
        ASSERT_EQ(timers.size(), 1) << "Invalid number of timers";
        unregisterPeriodicFunction("SomeFunction3");
        ASSERT_EQ(timers.size(), 0) << "Invalid number of timers";
    }

    void testRegisterUnregisterScheduled()
    {
        registerScheduledFunction("SomeFunctionScheduled", Time::now() + Duration::seconds(3), &TestModule::someFunction, this);
        registerScheduledFunction("SomeFunctionScheduled2", Time::now() + Duration::seconds(3), &TestModule::someFunction, this);
        ASSERT_EQ(timers.size(), 2) << "Invalid number of timers";
        registerScheduledFunction("SomeFunctionScheduled3", Time::now() + Duration::seconds(123), &TestModule::someFunction, this);
        ASSERT_EQ(timers.size(), 3) << "Invalid number of timers";
        registerScheduledFunction("SomeFunctionScheduled4", Time::now() + Duration::seconds(123), &TestModule::someFunction, this);
        ASSERT_EQ(timers.size(), 4) << "Invalid number of timers";

        unregisterPeriodicFunction("SomeFunctionScheduled2");
        ASSERT_EQ(timers.size(), 3) << "Invalid number of timers";
        unregisterPeriodicFunction("SomeFunctionScheduled");
        ASSERT_EQ(timers.size(), 2) << "Invalid number of timers";
        unregisterPeriodicFunction("SomeFunctionScheduled4");
        ASSERT_EQ(timers.size(), 1) << "Invalid number of timers";
        unregisterPeriodicFunction("SomeFunctionScheduled3");
        ASSERT_EQ(timers.size(), 0) << "Invalid number of timers";
    }

    void testRegisterUnregisterBasedOnSchedule()
    {
        // Create the Schedule object
        Schedule schedule;

        // Add 5 periodic entries
        for (int i = 0; i < 5; i++) {
            SchedulePeriodic* periodic_entry = schedule.add_periodic();
            ScheduleTimeOfDay* first_exec_time = periodic_entry->mutable_first_execution_time_of_day();
            first_exec_time->set_hour(8 + i);   // Set hour: 8, 9, 10, 11, 12
            first_exec_time->set_minute(0);
            first_exec_time->set_second(0);

            ScheduleTimeWindow* time_window = periodic_entry->mutable_only_active_between_time_frame();
            ScheduleTimeOfDay* start_time = time_window->mutable_start_time_of_day();
            start_time->set_hour(9);
            start_time->set_minute(0);
            start_time->set_second(0);

            ScheduleTimeOfDay* stop_time = time_window->mutable_stop_time_of_day();
            stop_time->set_hour(17);
            stop_time->set_minute(0);
            stop_time->set_second(0);

            // Set interval (in seconds for this example)
            periodic_entry->set_period_seconds(60 * (i + 1)); // Period: 60, 120, 180, etc. seconds
        }

        // Add 5 timed entries
        for (int i = 0; i < 5; i++) {
            ScheduleExactTime* timed_entry = schedule.add_timed();
            ScheduleTimeOfDay* time_of_day = timed_entry->mutable_time_of_day();
            time_of_day->set_hour(12);          // Set hour: 12 for all entries
            time_of_day->set_minute(30 + i);    // Minutes: 30, 31, 32, etc.
            time_of_day->set_second(0);

            // Repeat every day (1.0 day)
            timed_entry->set_repeat_every_n_days(1.0);
        }
        std::vector<std::string> functionNames = registerFunctionBasedOnSchedule("SomeFunctionSchedule", schedule, &TestModule::someFunction, this);

        ASSERT_EQ(timers.size(), 10) << "Invalid number of timers";
        for(int i = 0; i < 10; i++)
        {
            unregisterPeriodicFunction(functionNames[i]);
            ASSERT_EQ(timers.size(), 10 - (i + 1)) << "Invalid number of timers";
        }
    }

    void someFunction()
    {

    }

    void endTest()
    {
        moduleInfo("End test called");
        
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
    ASSERT_TRUE(finished);

    claid.shutdown();
}
