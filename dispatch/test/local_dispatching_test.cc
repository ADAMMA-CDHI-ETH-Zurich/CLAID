#include "dispatch/test/testmessages.pb.h"
#include "dispatch/core/local_dispatching.hh"
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


string msgToString(const Message& msg) {
    string buf;
    if (TextFormat::PrintToString(msg, &buf)) {
        return buf;
    }

    return "Message not valid (partial content: " +
            msg.ShortDebugString();
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
    cout << "checkpoint 1" << endl;

    for(auto& m : testModules) {
        modTable.setModule(get<0>(m), get<1>(m), get<2>(m));
    }
    cout << "checkpoint 2" << endl;

    for(auto pkt : testChannels) {
        modTable.setChannel(pkt->channel(), pkt->source_host_module(), pkt->target_host_module());
    }
    cout << "checkpoint 3" << endl;

    // modTable.moduleRuntimeMap = modRTMap;
    // modTable.runtimeModuleMap = rtModMap;
    // RuntimeQueueMap runtimeQueueMap; // map from runtime to outgoing queue
    // ChannelMap channelMap; // map from <channel_id, src, tgt> to DataPacket (= data type of channel )


    // std::map<std::string, claidservice::Runtime>  moduleRuntimeMap;  // map module ==> runtime
    // std::map<claidservice::Runtime, std::unordered_set<std::string>> runtimeModuleMap;  // map runtime to set of modules
    // RuntimeQueueMap runtimeQueueMap; // map from runtime to outgoing queue
    // ChannelMap channelMap; // map from <channel_id, src, tgt> to DataPacket (= data type of channel )


    cout << "checkpoint 4" << endl;

    DispatcherServer server(addr, modTable);
    ASSERT_TRUE(server.start()) << "Unable to start server";


    cout << "checkpoint 5" << endl;

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
    cout << "checkpoint 6" << endl;

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

    cout << "checkpoint 7" << endl;

    auto modListResponse = client.getModuleList();
    cout << "checkpoint 8" << endl;

    // Print out the proto messages:

    // cout << "expected:" << msgToString(expResp) << endl;
    // cout << "actual  :" << msgToString(*modListResponse) << endl;

    // ASSERT a certain module list
    auto diff = cmpMsg(expResp, *modListResponse);
    ASSERT_TRUE(diff == "") << diff;
//    ASSERT_TRUE(MessageDifferencer::Equals(expModListResponse, *modListResponse));
    cout << "checkpoint 9" << endl;

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
            if ((pkt->source_host_module() == modId) || (pkt->target_host_module() == modId)) {
                *newMod->add_channel_packets() = *pkt;
            }
        }
    }

    cout << "---------------------" << endl
        << msgToString(initReq) << endl
        << "-----------------------------" << endl;

    auto clientStarted = client.startRuntime(initReq);
    cout << "checkpoint 21a" << endl;

    ASSERT_TRUE(clientStarted) << "Unable to start client !";
    cout << "checkpoint 21b" << endl;

    cout << "checkpoint 22" << endl;

    // // Send packages to the out queue
    // shared_ptr<DataPackage> pkt;

    // outQueue.push_back(pkt);
    cout << "checkpoint 23" << endl;

    auto pkt1 = make_shared<DataPackage>(*chan12Pkt);
    pkt1->mutable_number_array_val()->add_val(101.0);
    pkt1->mutable_number_array_val()->add_val(102.5);
    pkt1->mutable_number_array_val()->add_val(12.3);
    outQueue.push_back(pkt1);

    cout << "checkpoint 24" << endl;

    auto pkt2 =  make_shared<DataPackage>(*chan13NumPkt);
    pkt2->set_number_val(pkt2->number_val()+100.0);
    outQueue.push_back(pkt2);

    auto pkt3 =  make_shared<DataPackage>(*chan23ProtoPkt);
    outQueue.push_back(pkt3);

    cout << "checkpoint 25" << endl;

    cout << "Sleeping for 3000 ms " << endl;
    std::this_thread::sleep_for(3000ms);


    cout << "checkpoint 26" << endl;

    // Expect those packages from the input queue.
    auto gotPkt1 = inQueue.pop_front();
    diff = cmpMsg(*pkt1, *gotPkt1);
    ASSERT_TRUE(diff == "") << diff;

    auto gotPkt2 = inQueue.pop_front();
    diff = cmpMsg(*pkt2, *gotPkt2);
    ASSERT_TRUE(diff == "") << diff;

    auto gotPkt3 = inQueue.pop_front();
    diff = cmpMsg(*pkt3, *gotPkt3);
    ASSERT_TRUE(diff == "") << diff;

    ExamplePayload gotExPayload;
    ASSERT_TRUE(parseFromBlob(gotPkt3->blob_val(), gotExPayload));

    diff = cmpMsg(*examplePayload, gotExPayload);
    ASSERT_TRUE(diff == "") << diff;

    cout << "Sleeping some more ! " << endl;
    std::this_thread::sleep_for(5000ms);

    // ASSERT_EQ(pkt3, outQueue.pop_front());
    modTable.inputQueue().close();
    routerThread->join();
}
