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
    *ret->mutable_source_host_module() = src;
    *ret->mutable_target_host_module() = tgt;
    setter(*ret); 
    return ret; 
}

TEST(RouterTestSuite, LocalRouterTest) 
{
    ModuleTable table;
    const std::string host = "host:";
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

    // Register Modules to different runtimes to simulate routing to different queues.
    table.setModule(mod1, "TestModuleClass1", {});
    table.setModule(mod2, "TestModuleClass2", {});
    table.setModule(mod3, "TestModuleClass3", {});

    SharedQueue<DataPackage>* outputQueue1 = table.lookupOutputQueue(mod1);
    ASSERT_NE(outputQueue1, nullptr) << "Could not find output queue for Module " << mod1;

    SharedQueue<DataPackage>* outputQueue2 = table.lookupOutputQueue(mod2);
    ASSERT_NE(outputQueue2, nullptr) << "Could not find output queue for Module " << mod2;

    SharedQueue<DataPackage>* outputQueue3 = table.lookupOutputQueue(mod3);
    ASSERT_NE(outputQueue3, nullptr) << "Could not find output queue for Module " << mod3;

    LocalRouter localRouter(table);

    // Simulate each Module sending to data to every Module (including itself).
    std::vector<std::shared_ptr<DataPackage>> packages = { 
        makePkt(channel11, absl::StrCat(host, mod1), absl::StrCat(host, mod1), [](auto p) { p.set_string_val("package11"); }),
        makePkt(channel12, absl::StrCat(host, mod1), absl::StrCat(host, mod2), [](auto p) { p.set_string_val("package12"); }),
        makePkt(channel13, absl::StrCat(host, mod1), absl::StrCat(host, mod3), [](auto p) { p.set_string_val("package13"); }),
        makePkt(channel21, absl::StrCat(host, mod2), absl::StrCat(host, mod1), [](auto p) { p.set_string_val("package21"); }),
        makePkt(channel22, absl::StrCat(host, mod2), absl::StrCat(host, mod2), [](auto p) { p.set_string_val("package22"); }),
        makePkt(channel23, absl::StrCat(host, mod2), absl::StrCat(host, mod3), [](auto p) { p.set_string_val("package23"); }), 
        makePkt(channel31, absl::StrCat(host, mod3), absl::StrCat(host, mod1), [](auto p) { p.set_string_val("package31"); }), 
        makePkt(channel32, absl::StrCat(host, mod3), absl::StrCat(host, mod2), [](auto p) { p.set_string_val("package32"); }), 
        makePkt(channel33, absl::StrCat(host, mod3), absl::StrCat(host, mod3), [](auto p) { p.set_string_val("package33"); })
    };

    for(std::shared_ptr<DataPackage>& package : packages)
    {
        absl::Status status = localRouter.routePackage(package);
        ASSERT_EQ(status.ok(), true) << status;
    }

    //ASSERT_EQ(outputQueue1->size(), 3) << "Expected 3 packages in output queue 1, but got " << outputQueue1->size();
    
}

TEST(RouterTestSuite, MasterRouterTest) 
{
    
}