#include "dispatch/core/Module/TypeMapping/TypeMapping.hh"
#include "gtest/gtest.h"

#include "dispatch/proto/claidservice.grpc.pb.h"
#include "dispatch/core/RemoteFunction/RemoteFunction.hh"
#include "dispatch/core/RemoteFunction/RemoteFunctionRunnableHandler.hh"
#include "dispatch/core/CLAID.hh"
using claidservice::DataPackage;

#include <string>
#include <vector>
#include <map>

using namespace claid;

bool rpcCorrect = false;
std::string errorMessage;

std::map<std::string, std::string> runningModulesRPCResult;

class TestClass
{
    private:

    public:
  

        std::string test(int a)
        {
            return std::to_string(a);
        }
};


class RPCCaller : public claid::Module
{


    void initialize(Properties properties)
    {
        moduleInfo("Intialize");
        registerScheduledFunction("TestModuleRPC", Time::now() + Duration::seconds(1), &RPCCaller::testModuleRPC, this);
        registerScheduledFunction("TestRuntimeRPC", Time::now() + Duration::seconds(1), &RPCCaller::testRuntimeRPC, this);
    }

    void testModuleRPC()
    {
        RemoteFunction<int> mappedFunction = mapRemoteFunctionOfModule<int, std::string, std::string>("RPCCallee", "calculate_sum");
        int result = mappedFunction.execute<std::string, std::string>("5", "42")->await();
        Logger::logInfo("Got result %d", result);
        if(result == 5 + 42)
        {
            rpcCorrect = true;
        }
        else
        {
            errorMessage = "Invalid result! Expected 47 but got " + std::to_string(result);
        }
    }

    void testRuntimeRPC()
    {
        RemoteFunction<std::map<std::string, std::string>> function = 
            mapRemoteFunctionOfRuntime<std::map<std::string, std::string>>(Runtime::RUNTIME_CPP, "get_all_running_modules_of_all_runtimes");
        
        runningModulesRPCResult = function.execute()->await();
    }

    
};

class RPCCallee : public claid::Module
{
    void initialize(Properties properties)
    {
        registerRemoteFunction("calculate_sum", &RPCCallee::calculateSum, this);
    }

    int calculateSum(std::string a, std::string b)
    {
        Logger::logInfo("Received data %s %s", a.c_str(), b.c_str());
        int result = std::atoi(a.c_str()) + std::atoi(b.c_str());
        Logger::logInfo("Result %d", result);
        return result;
    }
};

REGISTER_MODULE_FACTORY_CUSTOM_NAME(RPCCaller, RPCCaller)
REGISTER_MODULE_FACTORY_CUSTOM_NAME(RPCCallee, RPCCallee)

// Tests if all available Mutators can be implemented using the template specialization.
TEST(RemoteFunctionTestSuite, RemoteFunctionTest) 
{   
    FutureHandler handler;
    SharedQueue<DataPackage> queue;
    RemoteFunctionIdentifier identifier;
    RemoteFunction<std::string> function = makeRemoteFunction<std::string, int, int>(&handler, &queue, identifier);
    
    RemoteFunctionRunnableHandler runnableHandler("TestObject", queue);
    
    TestClass testObject;
    runnableHandler.registerRunnable("test", &TestClass::test, &testObject);

    const char* socket_path_local_1 = "unix:///tmp/remote_function_test_grpc_client.sock";


    const char* config_file = "dispatch/test/remote_function_test.json";
    const char* client_host_id = "test_client";
    const char* user_id = "user42";
    const char* device_id = "something_else";


    CLAID clientMiddleware;
    bool result = clientMiddleware.start(socket_path_local_1, config_file, client_host_id, user_id, device_id);
    ASSERT_TRUE(result) << "Failed to start CLAID";
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    clientMiddleware.shutdown();

    ASSERT_TRUE(rpcCorrect) << errorMessage;
    ASSERT_EQ(runningModulesRPCResult.size(), 2);
    ASSERT_EQ(runningModulesRPCResult["RPCCaller"], "RPCCaller");
    ASSERT_EQ(runningModulesRPCResult["RPCCallee"], "RPCCallee");

    std::cout << "Got running Modules: ";
    for(auto& entry : runningModulesRPCResult)
    {
        std::cout << entry.first << "(" << entry.second << ")\n";
    }

}