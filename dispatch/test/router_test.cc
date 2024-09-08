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

#include "gtest/gtest.h"
#include "dispatch/core/Router/MasterRouter.hh"
#include "dispatch/core/Router/LocalRouter.hh"
#include "dispatch/core/Router/ServerRouter.hh"
#include "dispatch/core/Router/ClientRouter.hh"

#include "dispatch/core/Router/RoutingTree.hh"
#include "dispatch/core/Router/RoutingTreeParser.hh"
#include "dispatch/test/test_helpers.hh"


#include <vector>
#include <memory>
#include <string>

using namespace claid;
using namespace claidservice;

using claidtest::setStringVal;

typedef std::function<void(DataPackage&)> setterFn;

std::shared_ptr<claidservice::DataPackage> makePkt(const std::string& channel, const std::string& src, const std::string& tgt, setterFn setter) {
    auto ret = std::make_shared<claidservice::DataPackage>();
    *ret->mutable_channel() = channel;
    *ret->mutable_source_module() = src;
    *ret->mutable_target_module() = tgt;
    setter(*ret.get());
    return ret;
}


const std::string mod1 = "TestModule1";
const std::string mod2 = "TestModule2";
const std::string mod3 = "TestModule3";

const std::string channel11 = "TestChannel11";
const std::string channel12 = "TestChannel12";
const std::string channel13 = "TestChannel13";
const std::string channel21 = "TestChannel21";
const std::string channel22 = "TestChannel22";
const std::string channel23 = "TestChannel23";
const std::string channel31 = "TestChannel31";
const std::string channel32 = "TestChannel32";
const std::string channel33 = "TestChannel33";

const std::string test_host = "test_host";

// Simulate each Module sending to data to every Module (including itself).
std::vector<std::shared_ptr<DataPackage>> packages = {
    makePkt(channel11, mod1, mod1, [](auto& p) {  p.set_target_host(test_host); setStringVal(p, "package11");}),
    makePkt(channel12, mod1, mod2, [](auto& p) { p.set_target_host(test_host); setStringVal(p, "package12"); }),
    makePkt(channel13, mod1, mod3, [](auto& p) { p.set_target_host(test_host); setStringVal(p, "package13"); }),
    makePkt(channel21, mod2, mod1, [](auto& p) { p.set_target_host(test_host); setStringVal(p, "package21"); }),
    makePkt(channel22, mod2, mod2, [](auto& p) { p.set_target_host(test_host); setStringVal(p, "package22"); }),
    makePkt(channel23, mod2, mod3, [](auto& p) { p.set_target_host(test_host); setStringVal(p, "package23"); }),
    makePkt(channel31, mod3, mod1, [](auto& p) { p.set_target_host(test_host); setStringVal(p, "package31"); }),
    makePkt(channel32, mod3, mod2, [](auto& p) { p.set_target_host(test_host); setStringVal(p, "package32"); }),
    makePkt(channel33, mod3, mod3, [](auto& p) { p.set_target_host(test_host); setStringVal(p, "package33"); })
};

std::shared_ptr<DataPackage> makePackageFromToHost(const std::string& source_host, const std::string& targetHost, const std::string& packageStringVal)
{
    std::shared_ptr<DataPackage> dataPackage = std::make_shared<DataPackage>();

    dataPackage->set_source_host(source_host); 
    dataPackage->set_target_host(targetHost); 
    setStringVal(*dataPackage, packageStringVal);
    return dataPackage;
}

void prepareModuleTableAndQueues(ModuleTable& table, SharedQueue<DataPackage>*& outputQueue1, SharedQueue<DataPackage>*& outputQueue2, SharedQueue<DataPackage>*& outputQueue3)
{  
    // Register Modules to different runtimes to simulate routing to different queues.
    table.setNeededModule(mod1, "TestModuleClass1", {});
    table.addModuleToRuntime(mod1, claidservice::Runtime::RUNTIME_CPP);
    table.setNeededModule(mod2, "TestModuleClass2", {});
    table.addModuleToRuntime(mod2, claidservice::Runtime::RUNTIME_JAVA);
    table.setNeededModule(mod3, "TestModuleClass3", {});
    table.addModuleToRuntime(mod3, claidservice::Runtime::RUNTIME_PYTHON);

    outputQueue1 = table.lookupOutputQueue(mod1);
    ASSERT_NE(outputQueue1, nullptr) << "Could not find output queue for Module " << mod1;

    outputQueue2 = table.lookupOutputQueue(mod2);
    ASSERT_NE(outputQueue2, nullptr) << "Could not find output queue for Module " << mod2;

    outputQueue3 = table.lookupOutputQueue(mod3);
    ASSERT_NE(outputQueue3, nullptr) << "Could not find output queue for Module " << mod3;
}

void assertQueues(SharedQueue<DataPackage>*& outputQueue1, SharedQueue<DataPackage>*& outputQueue2, SharedQueue<DataPackage>*& outputQueue3)
{
    ASSERT_EQ(outputQueue1->size(), 3) << "Expected 3 packages in output queue 1, but got " << outputQueue1->size();
    ASSERT_EQ(outputQueue2->size(), 3) << "Expected 3 packages in output queue 2, but got " << outputQueue2->size();
    ASSERT_EQ(outputQueue3->size(), 3) << "Expected 3 packages in output queue 3, but got " << outputQueue3->size();

    ASSERT_EQ(claidtest::getStringVal(*outputQueue1->pop_front()), "package11");
    ASSERT_EQ(claidtest::getStringVal(*outputQueue1->pop_front()), "package21");
    ASSERT_EQ(claidtest::getStringVal(*outputQueue1->pop_front()), "package31");

    ASSERT_EQ(claidtest::getStringVal(*outputQueue2->pop_front()), "package12");
    ASSERT_EQ(claidtest::getStringVal(*outputQueue2->pop_front()), "package22");
    ASSERT_EQ(claidtest::getStringVal(*outputQueue2->pop_front()), "package32");

    ASSERT_EQ(claidtest::getStringVal(*outputQueue3->pop_front()), "package13");
    ASSERT_EQ(claidtest::getStringVal(*outputQueue3->pop_front()), "package23");
    ASSERT_EQ(claidtest::getStringVal(*outputQueue3->pop_front()), "package33");
}

inline HostDescription makeHostDescription(const std::string& hostname, const std::string& hostServerAddress, const std::string& connectTo)
{
    HostConfig hostConfig;
    hostConfig.set_hostname(hostname);

    ServerConfig& serverConfig = *hostConfig.mutable_server_config();
    serverConfig.set_host_server_address(hostServerAddress);

    ClientConfig& clientConfig = *hostConfig.mutable_connect_to();
    clientConfig.set_host(connectTo);
    HostDescription description(hostConfig);
    return description;
}

inline void addHostDescription(HostDescriptionMap& map, const std::string& hostname, bool isServer, const std::string& hostServerAddress, const std::string& connectTo)
{
   absl::Status status = map.insert(std::make_pair(hostname, makeHostDescription(hostname, hostServerAddress, connectTo)));
   ASSERT_EQ(status.ok(), true) << status;
}


TEST(RouterTestSuite, LocalRouterTest)
{
    ModuleTable table;
    
    SharedQueue<DataPackage>* outputQueue1;
    SharedQueue<DataPackage>* outputQueue2;
    SharedQueue<DataPackage>* outputQueue3;

    prepareModuleTableAndQueues(table, outputQueue1, outputQueue2, outputQueue3);

    LocalRouter localRouter(test_host, table);

    for(std::shared_ptr<DataPackage>& package : packages)
    {
        absl::Status status = localRouter.routePackage(package);
        ASSERT_EQ(status.ok(), true) << status;
    }

    assertQueues(outputQueue1, outputQueue2, outputQueue3);
}


void assertHostUserTableQueue(HostUserTable& hostUserTable, const std::string& targetHost)
{
    absl::Status status;
    std::vector<std::shared_ptr<SharedQueue<DataPackage>>> queues;
    std::shared_ptr<SharedQueue<DataPackage>> queue;
    std::shared_ptr<DataPackage> package;


    status = hostUserTable.lookupOutputQueuesForHost(targetHost, queues);
    ASSERT_TRUE(status.ok()) << status;
    ASSERT_EQ(queues.size(), 1) << "Expected exactly one Queue to " << targetHost << ", however got " << queues.size();
    queue = queues[0];
    ASSERT_EQ(queue->size(), 1) << "Expected queue for " << targetHost << " to have exactly one package in the queue, but got " << queue->size();
    package = queue->pop_front();
    ASSERT_EQ(package->target_host(), targetHost);
}

// Tests the ServerRouter.
// Verifies that the ServerRouter outputs packages to the correct queues of directly connected and indirectly connected clients.
// Indirectly connected clients are clients that connect via another intermediate Router, e.g. Client1 -> Server1 -> Server2.
// The following test creates 3 Server and 3 client hosts, which are all directly and indirectly connected to Server1.
// The ServerRouter is configured to run on Server1. The test verifies that the ServerRouter routes all packages accordingly to all connected clients.
TEST(RouterTestSuite, ServerRouterTest)
{   
        std::cout << "T0\n";

    SharedQueue<DataPackage>* outputQueue1;
    SharedQueue<DataPackage>* outputQueue2;
    SharedQueue<DataPackage>* outputQueue3;

    HostDescriptionMap hostDescriptions;
    addHostDescription(hostDescriptions, "Server1", true, "claid1.ethz.ch", "");
    addHostDescription(hostDescriptions, "Client11", false, "claid3.claid.ethz.ch", "Server1");
    addHostDescription(hostDescriptions, "Client12", false, "claid3.claid.ethz.ch", "Server1");
    addHostDescription(hostDescriptions, "Server2", true, "claid2.ethz.ch", "Server1");
    addHostDescription(hostDescriptions, "Client2", false, "claid3.claid.ethz.ch", "Server2");
    addHostDescription(hostDescriptions, "Server3", true, "claid3.claid.ethz.ch", "Server2");
    addHostDescription(hostDescriptions, "Client3", false, "claid3.claid.ethz.ch", "Server3");


    const std::string currentHost = "Server1";
    RoutingTree routingTree;
    RoutingTreeParser routingTreeParser;

    absl::Status status;
    status = routingTreeParser.buildRoutingTree(hostDescriptions, routingTree);

    ASSERT_TRUE(status.ok()) << status;


    HostUserTable hostUserTable;
    
    // We are Server1, hence in our HostUserTable we only have clients/servers directly connected to us.
    status = hostUserTable.addRemoteClient("Server1", "UserID01", "DeviceID01");
    ASSERT_TRUE(status.ok()) << status;

    status = hostUserTable.addRemoteClient("Client11", "UserID02", "DeviceID02");
    ASSERT_TRUE(status.ok()) << status;

    status = hostUserTable.addRemoteClient("Client12", "UserID03", "DeviceID02");
    ASSERT_TRUE(status.ok()) << status;

    status = hostUserTable.addRemoteClient("Server2", "UserID04", "DeviceID03");
    ASSERT_TRUE(status.ok()) << status;

    
    ServerRouter serverRouter(currentHost, routingTree, hostUserTable);

    std::vector<std::shared_ptr<DataPackage>> serverPackages =
    {
        makePackageFromToHost("Server1", "Server2", "packageToServer2"),
        makePackageFromToHost("Server1", "Server3", "packageToServer3"),
        makePackageFromToHost("Server1", "Client11", "packageToClient1"),
        makePackageFromToHost("Server1", "Client12", "packageToClient12"),
        makePackageFromToHost("Server1", "Client2", "packageToClient2"),
        makePackageFromToHost("Server1", "Client3", "packageToClient3"),
    };

    for(std::shared_ptr<DataPackage> package : serverPackages)
    {
        serverRouter.routePackage(package);
    }


    std::vector<std::shared_ptr<SharedQueue<DataPackage>>> queues;

    status = hostUserTable.lookupOutputQueuesForHost("Server2", queues);
    ASSERT_TRUE(status.ok()) << status;

    
    ASSERT_EQ(queues.size(), 1) << "Expected exactly one Queue to Server2, however got " << queues.size();
    std::shared_ptr<SharedQueue<DataPackage>> queue = queues[0];

    // Packages for Server2, Client2, Server3 and Client3 are all routed via Server2
    // Hence, we expect 4 packages.
    ASSERT_EQ(queue->size(), 4) << "Expected queue for Server2 to have exactly one package in the queue, but got " << queue->size();
    

    std::vector<std::string> expectedTargetHosts = {"Server2", "Server3", "Client2", "Client3"};

    std::shared_ptr<DataPackage> package;
    for(const std::string& expectedHost : expectedTargetHosts)
    {
        package = queue->pop_front();
        ASSERT_EQ(package->target_host(), expectedHost) 
            << "Invalid package, expected target host " << expectedHost << " but got " << package->target_host();
    }

    assertHostUserTableQueue(hostUserTable, "Client11");
    assertHostUserTableQueue(hostUserTable, "Client12");

    status = hostUserTable.lookupOutputQueuesForHost("Server3", queues);
    ASSERT_FALSE(status.ok()) << "Expected there to be no Queue to Server3, however got " << queues.size() << " queues";;
    
    status = hostUserTable.lookupOutputQueuesForHost("Client3", queues);
    ASSERT_FALSE(status.ok()) << "Expected there to be no Queue to Server3, however got " << queues.size() << " queues";;
    
    
    
  //  assertQueues(outputQueue1, outputQueue2, outputQueue3);
}

TEST(RouterTestSuite, MasterRouterTest)
{
    ModuleTable table;
    
    SharedQueue<DataPackage>* outputQueue1;
    SharedQueue<DataPackage>* outputQueue2;
    SharedQueue<DataPackage>* outputQueue3;

    prepareModuleTableAndQueues(table, outputQueue1, outputQueue2, outputQueue3);

    std::shared_ptr<LocalRouter> localRouter = std::make_shared<LocalRouter>(test_host, table);

    SharedQueue<DataPackage> queue;
    HostDescriptionMap hostDescriptions;
    HostDescription hostDescription = makeHostDescription(test_host, "", "");

    hostDescriptions[test_host] = hostDescription;

    ModuleDescriptionMap moduleDescriptions;
    moduleDescriptions[mod1] = ModuleDescription(mod1, "ModClass1", test_host, {});
    moduleDescriptions[mod2] = ModuleDescription(mod2, "ModClass2", test_host, {});
    moduleDescriptions[mod3] = ModuleDescription(mod3, "ModClass3", test_host, {});


    MasterRouter masterRouter(test_host, "test_user", "test_device", hostDescriptions, moduleDescriptions, queue, localRouter);

    absl::Status status = masterRouter.start();
    if(status.ok())
    {
        std::cout << "Failed to start MasterRouter " << status << "\n";
    }

    for(std::shared_ptr<DataPackage>& package : packages)
    {
        Logger::logInfo("Pushing %s", package->target_host().c_str());
        queue.push_back(package);
    }
    
    status = masterRouter.stopAfterQueueFinished();
    ASSERT_TRUE(status.ok());
    assertQueues(outputQueue1, outputQueue2, outputQueue3);
}