#include "claid_main.hh"
#include "CLAID.hh"

int main()
{
    const std::string configurationPath = "/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/dispatch/test_config.json";
    const std::string currentHost = "alex_client";

    // This will be blocking, for now. Otherwise, we have to properly deal with the allocated resources.
    absl::Status status = claid::start(configurationPath, currentHost);
}