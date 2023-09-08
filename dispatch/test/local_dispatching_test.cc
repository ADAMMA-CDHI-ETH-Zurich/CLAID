#include "dispatch/core/local_dispatching.hh"
#include "gtest/gtest.h"
#include "google/protobuf/util/message_differencer.h"

using namespace std; 
using namespace claid;
using namespace claidservice;

using google::protobuf::util::MessageDifferencer;

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

void setModuleChannels(claidservice::InitRuntimeRequest::ModuleChannels modChannels, 
        const string& modId, const vector<pair<string, DataPackage&>>& chanTypes) {
            
}


const string 
    mod1 = "module_1", 
    mod2 = "module_2", 
    mod3 = "module_3",
    chan12 = "chan12", 
    chan13Num = "chan13-num",
    chan13Str = "chan13-str",
    chan23 = "chan23"; 

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


// define the packets for the different channels 
const shared_ptr<DataPackage> 
    chan12Pkt = makePkt(chan12, mod1, mod2, [](auto p) { p.mutable_number_array_val()->add_val(99); }),
    chan13NumPkt = makePkt(chan13Num, mod1, mod3, [](auto p) { p.set_number_val(42); }),
    chan13StrPkt = makePkt(chan13Str, mod1, mod3, [](auto p) { p.set_string_val("hitchhiker"); }),
    chan23Pkt = makePkt(chan12, mod1, mod2, [](auto p) { p.set_number_val(1042); }); 

// Define the test modules
typedef tuple<string, string, claidservice::Runtime, map<string, string>> ModInfo;
const ModInfo testModules[] = {
    make_tuple("module_1", "OneModuleClass", Runtime::RUNTIME_CPP, props_1),
    make_tuple("module_2", "TwoModuleClass", Runtime::RUNTIME_CPP, props_2),
    make_tuple("module_3", "ThreeModuleClass", Runtime::RUNTIME_CPP, props_3)
};

// Define the channel information 
typedef tuple<string, string, string, shared_ptr<DataPackage>> ChanInfo; 
const ChanInfo testChannels[] = {
    make_tuple(chan12, mod1, mod2, chan12Pkt),
    make_tuple(chan13Num, mod1, mod3, chan13NumPkt),
    make_tuple(chan13Str, mod1, mod3, chan13StrPkt),
    make_tuple(chan23, mod2, mod3, chan23Pkt),
}; 

TEST(LocalDispatcherTestSuit, SocketBasedDispatcherTest) {
    // TODO: remove this from being hard coded
    string addr = "unix:///tmp/claidtest/grpc.socket";
    
    // TODO: The module table needs to be populated. 
    ModuleTable modTable; 
    cout << "checkpoint 1" << endl;

    for(int i=0; i < sizeof(testModules)/sizeof(ModInfo); i++) {
        auto& m = testModules[i];
        modTable.setModule(get<0>(m), get<1>(m), get<2>(m), get<3>(m));
    }
    cout << "checkpoint 2" << endl;

    for(int i=0; i < sizeof(testChannels)/sizeof(ChanInfo); i++) {
        auto& m = testChannels[i];
        modTable.setChannel(get<0>(m), get<1>(m), get<2>(m));
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
    DispatcherClient client(addr, inQueue, outQueue);

    auto expModListResponse = ModuleListResponse();
    expModListResponse.add_descriptors();

    cout << "checkpoint 7" << endl;

    auto modListResponse = client.getModuleList(); 
    // cout << endl << *modListResponse << endl; 
    cout << "checkpoint 8" << endl;

    // ASSERT a certain module list 
    ASSERT_TRUE(MessageDifferencer::Equals(expModListResponse, *modListResponse)); 
    cout << "checkpoint 9" << endl;


    //   !google::protobuf::util::MessageDifferencer::Equals(
    //       *self->message, *reinterpret_cast<CMessage*>(other)->message)) {
    // https://github.com/search?q=repo%3Aprotocolbuffers%2Fprotobuf%20MessageDifferencer%3A%3AEqual&type=code
        


    // InitRuntimeRequest ir;
    // ir.set_runtime(Runtime::RUNTIME_CPP);
    // setModuleChannels(*ir.add_modules(), mod1, 
    //     {
    //         {chan12, chan12Pkt}
        
    //     });


    //     make_pair(chan12, chan12Pkt),
    //     make_pair(chan13Num, chan13NumPkt),
    //     make_pair(chan13Str, chan13StrPkt));
    // setModuleChannels(*ir.add_modules(), mod2, 
    //     make_pair(chan12, chan12Pkt),
    //     make_pair(chan23, chan23Pkt));
    // setModuleChannels(*ir.add_modules(), mod3, 
    //     make_pair(chan13Num, chan13NumPkt),
    //     make_pair(chan13Str, chan13StrPkt));
    //     make_pair(chan23, chan23Pkt));

    // ASSERT_TRUE(client.startRuntime(initReq)) << "Unable to start client !"; 

    // // Send packages to the out queue 
    // shared_ptr<DataPackage> pkt;

    // outQueue.push_back(pkt);


    // outQueue.push_back(pkt);
    // outQueue.push_back(pkt);
    // outQueue.push_back(pkt);

    // Sleep for a bit 

    // Expect those packages from the input queue. 
    // ASSERT_EQ(pkt1, outQueue.pop_front());
    // ASSERT_EQ(pkt2, outQueue.pop_front());
    // ASSERT_EQ(pkt3, outQueue.pop_front());
    modTable.inputQueue().close();
    routerThread->join(); 
}
