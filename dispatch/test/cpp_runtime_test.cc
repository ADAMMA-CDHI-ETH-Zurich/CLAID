#include "dispatch/core/Module/TypeMapping/TypeMapping.hh"
#include "gtest/gtest.h"

#include "dispatch/core/CLAID.hh"
using namespace claid;


class SenderModule : public Module
{
    Channel<std::string> channel;

    void initialize(const std::map<std::string, std::string>& properties)
    {
        Logger::logInfo("SenderModule init!");
        registerPeriodicFunction("TestFunction", &SenderModule::periodicFunction, this, Duration::milliseconds(100));
    }

    void periodicFunction()
    {
        Logger::logInfo("Periodic test.");
    }
};

class ReceiverModule : public Module
{

};

REGISTER_MODULE_FACTORY_CUSTOM_NAME(TestSenderModule, SenderModule)
REGISTER_MODULE_FACTORY_CUSTOM_NAME(TestReceiverModule, ReceiverModule)

// Creates two simple Modules (Sender, Receiver) and registers them with the C++ Runtime.
// Starts the C++ runtime and and waits for the Receiver to receive data from the Sender.
TEST(CppRuntimeTestSuite, CppRuntimeTest)  
{
    std::cout << "T1\n";
    const char* socket_path = "/tmp/test_grpc.sock";
    const char* config_file = "dispatch/test/cpp_runtime_test.json";
    const char* host_id = "alex_client";
    const char* user_id = "user42";
    const char* device_id = "something_else";
    std::cout << "T2\n";

    bool result = CLAID::start(socket_path, config_file, host_id, user_id, device_id);

    ASSERT_TRUE(result);
    std::cout << "Attach cpp runtime 4\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(6000));
    CLAID::shutdown();
}
