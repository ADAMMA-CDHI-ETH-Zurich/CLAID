#include "dispatch/test/test_helpers.hh"
#include "dispatch/core/local_dispatching.hh"
#include "gtest/gtest.h"

#include <chrono>
#include <thread>

using namespace std;
using namespace claid;
using namespace claidtest;
using namespace claidservice;
using namespace examplemsg;
using namespace std::chrono_literals;

using google::protobuf::Message;
using google::protobuf::TextFormat;
using google::protobuf::util::MessageDifferencer;
using google::protobuf::util::MessageDifferencer;
using google::protobuf::io::StringOutputStream;

TEST(LocalDispatcherTestSuite, SocketBasedDispatcherTest) {
    // TODO: remove this from being hard coded
    string addr = "unix:///tmp/claidtest-local-grpc.socket";

    ModuleTable modTable;
    for(auto& m : testModules) {
        modTable.setNeededModule(m.modId, m.modClass, m.props);
        modTable.setModuleChannelToConnectionMappings(m.modId, channelToConnectionMappings, channelToConnectionMappings);
    }

    for(auto pkt : testChannels) {
        modTable.setExpectedChannel(pkt->channel(), pkt->source_module(), pkt->target_module());

    }

    DispatcherServer server(addr, modTable);
    ASSERT_TRUE(server.start()) << "Unable to start server";

    auto routerThread = make_unique<thread>([&modTable]() {
        while(true) {
            Logger::logInfo("Routing 1");
            auto pkt = modTable.inputQueue().pop_front();
            Logger::logInfo("Routing 2 %d", modTable.inputQueue().size());
            if(!pkt)
            {
                Logger::logInfo("pkt is nullptr");
            }
            if (!pkt || modTable.inputQueue().is_closed()) {
                Logger::logInfo("discarding package");
                return;
            }
            auto outQ = modTable.lookupOutputQueue(pkt->target_module());
            if (outQ) {
                outQ->push_back(pkt);
            }
        }
    });

    SharedQueue<claidservice::DataPackage> inQueue;
    SharedQueue<claidservice::DataPackage> outQueue;

    // Not needed for this test, hence empty.
    std::map<std::string, claidservice::ModuleAnnotation> moduleAnnotations;
    
    // Create the client.
    DispatcherClient client(addr, inQueue, outQueue, supportedModClasses, moduleAnnotations);

    auto expResp = ModuleListResponse();
    for(auto& m : testModules) {
        auto desc = expResp.add_descriptors();
        desc->set_module_id(m.modId);
        desc->set_module_class(m.modClass);
        desc->mutable_properties()->insert(m.props.begin(), m.props.end());
    };

    auto modListResponse = client.getModuleList();

    // ASSERT a certain module list
    auto diff = cmpMsg(expResp, *modListResponse);
    ASSERT_TRUE(diff == "") << diff;

    //   !google::protobuf::util::MessageDifferencer::Equals(
    //       *self->message, *reinterpret_cast<CMessage*>(other)->message)) {
    // https://github.com/search?q=repo%3Aprotocolbuffers%2Fprotobuf%20MessageDifferencer%3A%3AEqual&type=code

    Logger::logInfo("local_dispatching_test 1");
    InitRuntimeRequest initReq;
    initReq.set_runtime(Runtime::RUNTIME_CPP);
    auto modulesRef = initReq.mutable_modules();
    for(auto& modInfo : testModules) {
        auto& modId = modInfo.modId;
        auto newMod = modulesRef->Add();
        newMod->set_module_id(modId);
        for(auto pkt : testChannels) {
            bool isSrc = pkt->source_module() == modId;
            bool isTgt = (pkt->target_module() == modId);
            if (isSrc || isTgt) {
                DataPackage cpPkt(*pkt);
                if (!isSrc) {
                    cpPkt.clear_source_module();
                }
                if (!isTgt) {
                    cpPkt.clear_target_module();
                }
                *newMod->add_channel_packets() = cpPkt;
            }
        }
    }
    Logger::logInfo("local_dispatching_test 2");

    Logger::logInfo("%s", messageToString(initReq).c_str());

    auto clientStarted = client.startRuntime(initReq);
    ASSERT_TRUE(clientStarted.ok()) << clientStarted.error_message();

    auto pkt12 = make_shared<DataPackage>(*chan12Pkt);
    pkt12->mutable_number_array_val()->add_val(101.0);
    pkt12->mutable_number_array_val()->add_val(102.5);
    pkt12->mutable_number_array_val()->add_val(12.3);
    outQueue.push_back(pkt12);

    auto pkt13 =  make_shared<DataPackage>(*chan13NumPkt);
    pkt13->set_number_val(pkt13->number_val()+100.0);
    outQueue.push_back(pkt13);

    auto pkt23 =  make_shared<DataPackage>(*chan23ProtoPkt);
    outQueue.push_back(pkt23);

    // Send packages on the channel where we can have multiple
    // senders and multiple receivers.
    map<tuple<string, string, string>, list<shared_ptr<DataPackage>>> receivePkts;

    outQueue.push_back(makeChanPacket(chan12To45, mod1, receivePkts, testChannels, [](auto& p) { p.set_number_val(1001); }));
    outQueue.push_back(makeChanPacket(chan12To45, mod2, receivePkts, testChannels, [](auto& p) { p.set_number_val(1002); }));
    outQueue.push_back(makeChanPacket(chan12To45, mod1, receivePkts, testChannels, [](auto& p) { p.set_number_val(1003); }));
    outQueue.push_back(makeChanPacket(chan12To45, mod2, receivePkts, testChannels, [](auto& p) { p.set_number_val(1004); }));
    Logger::logInfo("local_dispatching_test 3");

    std::this_thread::sleep_for(1000ms);
    Logger::logInfo("local_dispatching_test 4 %d", inQueue.size());

    // Expect those packages from the input queue.
    auto gotPkt12 = inQueue.pop_front();
    Logger::logInfo("local_dispatching_test 5");
    ASSERT_EQ(gotPkt12->source_module(), mod1);
    ASSERT_EQ(gotPkt12->target_module(), mod2);
    diff = cmpMsg(*pkt12, *gotPkt12);
    Logger::logInfo("local_dispatching_test 6");
    ASSERT_TRUE(diff == "") << diff;

    Logger::logInfo("local_dispatching_test 7");
    auto gotPkt13 = inQueue.pop_front();
    Logger::logInfo("local_dispatching_test 8");
    ASSERT_EQ(gotPkt13->source_module(), mod1);
    ASSERT_EQ(gotPkt13->target_module(), mod3);
    diff = cmpMsg(*pkt13, *gotPkt13);
    ASSERT_TRUE(diff == "") << diff;
    Logger::logInfo("local_dispatching_test 9");

    auto gotPkt23 = inQueue.pop_front();
    ASSERT_EQ(gotPkt23->source_module(), mod2);
    ASSERT_EQ(gotPkt23->target_module(), mod3);
    diff = cmpMsg(*pkt23, *gotPkt23);
    ASSERT_TRUE(diff == "") << diff;

    ExamplePayload gotExPayload;
    ASSERT_TRUE(parseFromBlob(gotPkt23->blob_val(), gotExPayload));

    diff = cmpMsg(*examplePayload, gotExPayload);
    ASSERT_TRUE(diff == "") << diff;
    Logger::logInfo("local_dispatching_test 5");

    while(inQueue.size() > 0) {
        Logger::logInfo("Queue");
        auto gotPkt = inQueue.pop_front();
        auto key = make_tuple(gotPkt->channel(), gotPkt->source_module(), gotPkt->target_module());
        auto pl = receivePkts.find(key);
        ASSERT_FALSE(pl == receivePkts.end());

        // Look for the packet in the list of packages.
        for(auto lit = pl->second.begin(); lit != pl->second.end(); pl++) {
            if ((gotPkt->channel() == (*lit)->channel()) &&
                (gotPkt->source_module() == (*lit)->source_module()) &&
                (gotPkt->target_module() == (*lit)->target_module())) {
                    // remove the packet from the list
                    pl->second.erase(lit);
                    break;
            }
        }

        if (pl->second.empty()) {
            receivePkts.erase(pl);
        }
    }

    // Make sure every incoming packet has been removed from the
    // packets we expected to receive.
    ASSERT_TRUE(receivePkts.empty());

    // TODO: Fix threading so the shutdown below no longer hangs.
    // Make sure to run the test with something like:
    // CC=clang bazel test --test_output=all --test_timeout=10  :local_dispatching_test
    //
    Logger::logInfo(" -----------------------------------------------------------------");
    Logger::logInfo("| Test passed - trying shutdown which will fail currently !       |");
    Logger::logInfo(" -----------------------------------------------------------------");

    Logger::logInfo("Shutdown");
    // ASSERT_EQ(pkt3, outQueue.pop_front());
    client.shutdown();

    // server.shutdown();
    modTable.inputQueue().close();
    routerThread->join();
}

