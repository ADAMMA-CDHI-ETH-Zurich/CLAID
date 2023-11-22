#include "gtest/gtest.h"
#include "dispatch/core/Router/MasterRouter.hh"
#include "dispatch/core/Router/LocalRouter.hh"
#include "dispatch/core/Router/ServerRouter.hh"
#include "dispatch/core/Router/ClientRouter.hh"

#include <vector>
#include <memory>
#include <string>

using namespace claid;

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
    makePkt(channel11, mod1, mod1, [](auto& p) {  p.set_target_host(test_host); p.set_string_val("package11");}),
    makePkt(channel12, mod1, mod2, [](auto& p) { p.set_target_host(test_host); p.set_string_val("package12"); }),
    makePkt(channel13, mod1, mod3, [](auto& p) { p.set_target_host(test_host); p.set_string_val("package13"); }),
    makePkt(channel21, mod2, mod1, [](auto& p) { p.set_target_host(test_host); p.set_string_val("package21"); }),
    makePkt(channel22, mod2, mod2, [](auto& p) { p.set_target_host(test_host); p.set_string_val("package22"); }),
    makePkt(channel23, mod2, mod3, [](auto& p) { p.set_target_host(test_host); p.set_string_val("package23"); }),
    makePkt(channel31, mod3, mod1, [](auto& p) { p.set_target_host(test_host); p.set_string_val("package31"); }),
    makePkt(channel32, mod3, mod2, [](auto& p) { p.set_target_host(test_host); p.set_string_val("package32"); }),
    makePkt(channel33, mod3, mod3, [](auto& p) { p.set_target_host(test_host); p.set_string_val("package33"); })
};

void prepareTableAndQueues(ModuleTable& table, SharedQueue<DataPackage>*& outputQueue1, SharedQueue<DataPackage>*& outputQueue2, SharedQueue<DataPackage>*& outputQueue3)
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

    ASSERT_EQ(outputQueue1->pop_front()->string_val(), "package11");
    ASSERT_EQ(outputQueue1->pop_front()->string_val(), "package21");
    ASSERT_EQ(outputQueue1->pop_front()->string_val(), "package31");

    ASSERT_EQ(outputQueue2->pop_front()->string_val(), "package12");
    ASSERT_EQ(outputQueue2->pop_front()->string_val(), "package22");
    ASSERT_EQ(outputQueue2->pop_front()->string_val(), "package32");

    ASSERT_EQ(outputQueue3->pop_front()->string_val(), "package13");
    ASSERT_EQ(outputQueue3->pop_front()->string_val(), "package23");
    ASSERT_EQ(outputQueue3->pop_front()->string_val(), "package33");
}

TEST(RouterTestSuite, LocalRouterTest)
{
    ModuleTable table;
    
    SharedQueue<DataPackage>* outputQueue1;
    SharedQueue<DataPackage>* outputQueue2;
    SharedQueue<DataPackage>* outputQueue3;

    prepareTableAndQueues(table, outputQueue1, outputQueue2, outputQueue3);

    LocalRouter localRouter(test_host, table);

    for(std::shared_ptr<DataPackage>& package : packages)
    {
        absl::Status status = localRouter.routePackage(package);
        ASSERT_EQ(status.ok(), true) << status;
    }

    assertQueues(outputQueue1, outputQueue2, outputQueue3);
}

TEST(RouterTestSuite, MasterRouterTest)
{
    ModuleTable table;
    
    SharedQueue<DataPackage>* outputQueue1;
    SharedQueue<DataPackage>* outputQueue2;
    SharedQueue<DataPackage>* outputQueue3;

    prepareTableAndQueues(table, outputQueue1, outputQueue2, outputQueue3);

    std::shared_ptr<LocalRouter> localRouter = std::make_shared<LocalRouter>(test_host, table);

    SharedQueue<DataPackage> queue;
    HostDescriptionMap hostDescriptions;
    HostDescription hostDescription;
    hostDescription.hostname = test_host;
    hostDescription.isServer = false;
    hostDescriptions[test_host] = hostDescription;

    ModuleDescriptionMap moduleDescriptions;
    moduleDescriptions[mod1] = ModuleDescription(mod1, "ModClass1", test_host, {});
    moduleDescriptions[mod2] = ModuleDescription(mod2, "ModClass2", test_host, {});
    moduleDescriptions[mod3] = ModuleDescription(mod3, "ModClass3", test_host, {});


    MasterRouter masterRouter(test_host, hostDescriptions, moduleDescriptions, queue, localRouter);

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