#include "claid_main.hh"
#include "CLAIDConfig/CLAIDConfig.hh"

#include "Router/Router.hh"
#include <iostream>

int main()
{
    std::string output;
    std::shared_ptr<claidservice::CLAIDConfig> configPtr;
    absl::Status status = claid::parseConfigFile("/Users/planger/Development/PortAIbleStandaloneTest/test_config.json", configPtr);
    std::cout << status << "\n";
    claid::configToJson(configPtr, output);
    std::cout << "hello world\n" << output << "\n";

    claid::Router router;
    router.buildRoutingTable("alex_client", *configPtr.get());
}