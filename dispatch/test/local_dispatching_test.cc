#include "dispatch/test/testmessages.pb.h"
#include "dispatch/core/Logger/Logger.hh"
#include "dispatch/core/local_dispatching.hh"
#include "dispatch/core/proto_util.hh"
#include "gtest/gtest.h"
#include "google/protobuf/util/message_differencer.h"

#include <chrono>
#include <thread>

#include <google/protobuf/text_format.h>


using namespace std;
using namespace claid;
using namespace claidservice;
using namespace examplemsg;

using namespace std::chrono_literals;

using google::protobuf::Message;
using google::protobuf::TextFormat;
using google::protobuf::util::MessageDifferencer;
using google::protobuf::util::MessageDifferencer;
using google::protobuf::io::StringOutputStream;


// module-->runtime
// runtime --> modules

// module, runtime, channels
// module, runtime, channels

// channel

typedef std::function<void(DataPackage&)> setterFn;

shared_ptr<claidservice::DataPackage> makePkt(const string& channel, const string& src, const string& tgt, setterFn setter) {
    auto ret = make_shared<claidservice::DataPackage>();
    *ret->mutable_channel() = channel;
    *ret->mutable_source_host_module() = src;
    *ret->mutable_target_host_module() = tgt;
    setter(*ret);
    return ret;
}

unique_ptr<ExamplePayload> mkExamplePayload() {
    auto ret = make_unique<ExamplePayload>();
    ret->set_name("john doe");
    ret->set_value(99);
    return ret;
}

void setModuleChannels(claidservice::InitRuntimeRequest::ModuleChannels modChannels,
        const string& modId, const vector<pair<string, DataPackage&>>& chanTypes) {

}

// Compare two protobuf and return the result.
string cmpMsg(const Message& m1, const Message& m2) {
    MessageDifferencer differ;
    differ.set_report_matches(false);
    differ.set_report_moves(false);
    string diffResult;

    // compound statement forces to close the reporter via the destructor
    {
        StringOutputStream outStream(&diffResult);
        MessageDifferencer::StreamReporter reporter(&outStream);
        reporter.SetMessages(m1, m2);
        differ.ReportDifferencesTo(&reporter);
        differ.Compare(m1, m2);
    }
  return diffResult;
}

const string
    mod1 = "module_1",
    mod2 = "module_2",
    mod3 = "module_3",
    chan12 = "chan12",
    chan13Num = "chan13-num",
    chan13Str = "chan13-str",
    chan23 = "chan23",
    chan23Proto = "chan23Proto",
    modClass1 = "OneModuleClass",
    modClass2 = "TwoModuleClass",
    modClass3 = "ThreeModuleClass";

const map<string, string>
    props_1 = {
        {"prop11", "val11"},
        {"prop12", "val12"}
    },
    props_2 = {
        {"prop21", "val21"}
    },
    props_3 = {
        {"prop31", "val31"}
    };


void clearSrcTgt(DataPackage& pkt) {
    pkt.clear_source_host_module();
    pkt.clear_target_host_module();
}

void blobMsgFromProto(const Message& msg, Blob& targetBlob) {
    targetBlob.set_codec(Codec::CODEC_PROTO);
    targetBlob.set_message_type(msg.GetDescriptor()->full_name());
    string payload;
    msg.SerializeToString(&payload);
    targetBlob.set_payload(std::move(payload));
}

bool parseFromBlob(const Blob& srcBlob, Message& msg) {
    if ((srcBlob.codec() != Codec::CODEC_PROTO) ||
        (srcBlob.message_type() != msg.GetDescriptor()->full_name()) ||
        (srcBlob.message_type().empty()) ||
        (srcBlob.payload().empty())) {
        return false;
    }

    return msg.ParseFromString(srcBlob.payload());
}

const auto examplePayload = mkExamplePayload();

// define the packets for the different channels
const shared_ptr<DataPackage>
    chan12Pkt = makePkt(chan12, mod1, mod2, [](auto& p) { p.mutable_number_array_val()->add_val(99); }),
    chan13NumPkt = makePkt(chan13Num, mod1, mod3, [](auto& p) { p.set_number_val(42); }),
    chan13StrPkt = makePkt(chan13Str, mod1, mod3, [](auto& p) { p.set_string_val("hitchhiker"); }),
    chan23Pkt = makePkt(chan23, mod2, mod3, [](auto& p) { p.set_number_val(1042); }),
    chan23ProtoPkt = makePkt(chan23Proto, mod2, mod3, [](auto& p) { blobMsgFromProto(*examplePayload, *p.mutable_blob_val()); });

// Define the test modules
typedef tuple<string, string, map<string, string>> ModInfo;
const vector<ModInfo> testModules = {
    make_tuple(mod1, modClass1, props_1),
    make_tuple(mod2, modClass2, props_2),
    make_tuple(mod3, modClass3, props_3)
};
const set<string> supportedModClasses = { modClass1, modClass2, modClass3 };

// Define the channel information
// <channel_name, module_name, channel_type_as_package>

const vector<shared_ptr<DataPackage>> testChannels = {
    chan12Pkt, chan13NumPkt, chan13StrPkt, chan23Pkt, chan23ProtoPkt
};


// typedef tuple<string, string, string, shared_ptr<DataPackage>> ChanInfo;
// const vector<ChanInfo> testChannels = {
//     make_tuple(chan12, mod1, mod2, chan12Pkt),
//     make_tuple(chan13Num, mod1, mod3, chan13NumPkt),
//     make_tuple(chan13Str, mod1, mod3, chan13StrPkt),
//     make_tuple(chan23, mod2, mod3, chan23Pkt),
// };

TEST(LocalDispatcherTestSuit, SocketBasedDispatcherTest) {
    // TODO: remove this from being hard coded
    string addr = "unix:///tmp/claidtest-local-grpc.socket";

    // TODO: The module table needs to be populated.
    ModuleTable modTable;
    for(auto& m : testModules) {
        modTable.setModule(get<0>(m), get<1>(m), get<2>(m));
    }

    for(auto pkt : testChannels) {
        modTable.setChannel(pkt->channel(), pkt->source_host_module(), pkt->target_host_module());
    }

    DispatcherServer server(addr, modTable);
    ASSERT_TRUE(server.start()) << "Unable to start server";

    auto routerThread = make_unique<thread>([&modTable]() {
        while(true) {
            auto pkt = modTable.inputQueue().pop_front();
            if (!pkt || modTable.inputQueue().is_closed()) {
                return;
            }
            auto outQ = modTable.lookupOutputQueue(pkt->target_host_module());
            if (outQ) {
                outQ->push_back(pkt);
            }
        }
    });

    SharedQueue<claidservice::DataPackage> inQueue;
    SharedQueue<claidservice::DataPackage> outQueue;

    // Create the client.
    DispatcherClient client(addr, inQueue, outQueue, supportedModClasses);

    auto expResp = ModuleListResponse();
    auto addFn = [&expResp](const string& modId, const string& modClass, const map<string,string>& props) {
        auto desc = expResp.add_descriptors();
        desc->set_module_id(modId);
        desc->set_module_class(modClass);
        desc->mutable_properties()->insert(props.begin(), props.end());
    };
    addFn(get<0>(testModules[0]), get<1>(testModules[0]), get<2>(testModules[0]));
    addFn(get<0>(testModules[1]), get<1>(testModules[1]), get<2>(testModules[1]));
    addFn(get<0>(testModules[2]), get<1>(testModules[2]), get<2>(testModules[2]));

    auto modListResponse = client.getModuleList();

    // ASSERT a certain module list
    auto diff = cmpMsg(expResp, *modListResponse);
    ASSERT_TRUE(diff == "") << diff;

    //   !google::protobuf::util::MessageDifferencer::Equals(
    //       *self->message, *reinterpret_cast<CMessage*>(other)->message)) {
    // https://github.com/search?q=repo%3Aprotocolbuffers%2Fprotobuf%20MessageDifferencer%3A%3AEqual&type=code

    InitRuntimeRequest initReq;
    initReq.set_runtime(Runtime::RUNTIME_CPP);
    auto modulesRef = initReq.mutable_modules();
    for(auto& modInfo : testModules) {
        auto& modId = get<0>(modInfo);
        auto newMod = modulesRef->Add();
        newMod->set_module_id(modId);
        for(auto pkt : testChannels) {
            bool isSrc = pkt->source_host_module() == modId;
            bool isTgt = (pkt->target_host_module() == modId);
            if (isSrc || isTgt) {
                DataPackage cpPkt(*pkt);
                if (!isSrc) {
                    cpPkt.clear_source_host_module();
                }
                if (!isTgt) {
                    cpPkt.clear_target_host_module();
                }
                *newMod->add_channel_packets() = cpPkt;
            }
        }
    }

    Logger::printfln("%s", messageToString(initReq).c_str());

    auto clientStarted = client.startRuntime(initReq);
    ASSERT_TRUE(clientStarted) << "Unable to start client !";

    auto pkt12 = make_shared<DataPackage>(*chan12Pkt);
    clearSrcTgt(*pkt12);
    pkt12->mutable_number_array_val()->add_val(101.0);
    pkt12->mutable_number_array_val()->add_val(102.5);
    pkt12->mutable_number_array_val()->add_val(12.3);
    outQueue.push_back(pkt12);

    auto pkt13 =  make_shared<DataPackage>(*chan13NumPkt);
    clearSrcTgt(*pkt13);
    pkt13->set_number_val(pkt13->number_val()+100.0);
    outQueue.push_back(pkt13);

    auto pkt23 =  make_shared<DataPackage>(*chan23ProtoPkt);
    clearSrcTgt(*pkt23);
    outQueue.push_back(pkt23);

    std::this_thread::sleep_for(1000ms);

    // Expect those packages from the input queue.
    auto gotPkt12 = inQueue.pop_front();
    ASSERT_EQ(gotPkt12->source_host_module(), mod1);
    ASSERT_EQ(gotPkt12->target_host_module(), mod2);
    clearSrcTgt(*gotPkt12);
    diff = cmpMsg(*pkt12, *gotPkt12);
    ASSERT_TRUE(diff == "") << diff;

    auto gotPkt13 = inQueue.pop_front();
    ASSERT_EQ(gotPkt13->source_host_module(), mod1);
    ASSERT_EQ(gotPkt13->target_host_module(), mod3);
    clearSrcTgt(*gotPkt13);
    diff = cmpMsg(*pkt13, *gotPkt13);
    ASSERT_TRUE(diff == "") << diff;

    auto gotPkt23 = inQueue.pop_front();
    ASSERT_EQ(gotPkt23->source_host_module(), mod2);
    ASSERT_EQ(gotPkt23->target_host_module(), mod3);
    clearSrcTgt(*gotPkt23);
    diff = cmpMsg(*pkt23, *gotPkt23);
    ASSERT_TRUE(diff == "") << diff;

    ExamplePayload gotExPayload;
    ASSERT_TRUE(parseFromBlob(gotPkt23->blob_val(), gotExPayload));

    diff = cmpMsg(*examplePayload, gotExPayload);
    ASSERT_TRUE(diff == "") << diff;

    // ASSERT_EQ(pkt3, outQueue.pop_front());
    client.shutdown();

    // server.shutdown();
    modTable.inputQueue().close();
    routerThread->join();
}
