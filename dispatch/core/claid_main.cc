#include "claid_main.hh"
#include "CLAIDConfig/CLAIDConfig.hh"

#include "Router/MasterRouter.hh"
#include <iostream>
#include "shared_queue.hh"
int main()
{
    std::string output;
    std::shared_ptr<claidservice::CLAIDConfig> configPtr;
    absl::Status status = claid::parseConfigFile("/Users/planger/Development/PortAIbleStandaloneTest/test_config.json", configPtr);
    std::cout << status << "\n";
    if(!claid::configToJson(configPtr, output).ok())
    {
        printf("Failed to parse config file\n");
        exit(0);
    }
    std::cout << "hello world\n" << output << "\n";

    // The local RuntimeDispatchers and the external server and client dispatcher
    // will all output their received packages to the master queue.
    // The master queue is processed by the MasterRouter, which forwards to
    // the LocalRouter, ServerRouter or ClientRouter as required. 
    SharedQueue<claidservice::DataPackage> masterQueue;

    // Pass the masterQueue to the lookup tables, so they all know where to put packages.
    ModuleTable localModuleTable(masterQueue);
    ServerTable serverTable(masterQueue);
    ClientTable clientTable(masterQueue);


    
    claid::MasterRouter router(masterQueue, localModuleTable, serverTable, clientTable);
    router.start();


    if(!router.buildRoutingTable("alex_client", *configPtr.get()).ok())
    {
        if(!claid::configToJson(configPtr, output).ok())
        {
            printf("Failed to build routing table\n");
            exit(0);
        }
    }
    
}