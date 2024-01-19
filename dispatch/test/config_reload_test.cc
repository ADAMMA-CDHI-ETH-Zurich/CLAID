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
    void initialize(const std::map<std::string, std::string>& propertiesMap)
    {
        auto it = propertiesMap.find("TestModule1Data");
        if(it != propertiesMap.end())
        {
            if(it->second == "42")
            {
                testModule1Started = true;
            }
        }
        else
        {
            Logger::logError("TestModule1 did not find property TestModule1Data");
        }
    }
};

REGISTER_MODULE(TestModule1, TestModule1);

class TestModule2 : public Module
{
    void initialize(const std::map<std::string, std::string>& propertiesMap)
    {
        auto it = propertiesMap.find("TestModule2Data");
        if(it != propertiesMap.end())
        {
            if(it->second == "1337")
            {
                testModule2Started = true;
            }
        }
        else
        {
            Logger::logError("TestModule2 did not find property TestModule2Data");
        }
    }
};

REGISTER_MODULE(TestModule2, TestModule2);


class TestModule3 : public Module
{
    void initialize(const std::map<std::string, std::string>& propertiesMap)
    {
        auto it = propertiesMap.find("TestModule3Data");
        Logger::logInfo("TestModule 3 initialize");
        if(it != propertiesMap.end())
        {
            if(it->second == "420")
            {
                testModule3Started = true;
            }
        }
        else
        {
            Logger::logError("TestModule3 did not find property TestModule3Data");
        }
    }
};

REGISTER_MODULE(TestModule3, TestModule3);


class TestModule4 : public Module
{
    void initialize(const std::map<std::string, std::string>& propertiesMap)
    {
        auto it = propertiesMap.find("TestModule4Data");
        if(it != propertiesMap.end())
        {
            if(it->second == "96")
            {
                testModule4Started = true;
            }
        }
        else
        {
            Logger::logError("TestModule4 did not find property TestModule4Data");
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

    claid.loadNewConfig("dispatch/test/config_reload_test_2.json");
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