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

#ifndef TEST_HELPERS_HH_
#define TEST_HELPERS_HH_

#include "dispatch/core/Logger/Logger.hh"
#include "dispatch/core/proto_util.hh"
#include "dispatch/proto/claidservice.grpc.pb.h"
#include "dispatch/test/testmessages.pb.h"

// #include "gtest/gtest.h"
#include <google/protobuf/text_format.h>
#include "google/protobuf/util/message_differencer.h"

// #include <chrono>
// #include <thread>



// using namespace std;
// using namespace claid;
// using namespace claidservice;
// using namespace examplemsg;
// using namespace std::chrono_literals;

// using google::protobuf::Message;
// using google::protobuf::TextFormat;
// using google::protobuf::util::MessageDifferencer;
// using google::protobuf::util::MessageDifferencer;
// using google::protobuf::io::std::stringOutputStream;

namespace claidtest {

// TODO: Maybe factor these out in a separate package.
// Forward definition of helper types and functions.
typedef std::function<void(DataPackage&)> setterFn;

// Define the test modules
typedef struct ModInfo {
    std::string modId;
    std::string modClass;
    google::protobuf::Struct props;
} ModInfo;


std::shared_ptr<claidservice::DataPackage> makePkt(const std::string& channel, const std::string& src, const std::string& tgt, setterFn setter);

// std::shared_ptr<clai dservice::DataPackage> makeChanPacket(const std::string&, const std::string&, std::vector<std::shared_ptr<DataPackage>>&,
//     const std::vector<std::shared_ptr<DataPackage>>&, setterFn setter);



std::shared_ptr<claidservice::DataPackage> makeChanPacket(const std::string& channel, const std::string& src,
        std::map<std::tuple<std::string, std::string, std::string>, std::list<std::shared_ptr<claidservice::DataPackage>>>& receivePkts,
        const std::vector<std::shared_ptr<claidservice::DataPackage>>& testChannels, setterFn setter);

std::unique_ptr<examplemsg::ExamplePayload> mkExamplePayload();
// Compare two protobuf and return the result.
std::string cmpMsg(const google::protobuf::Message& m1, const google::protobuf::Message& m2);
void clearSrcTgt(DataPackage& pkt);
void blobMsgFromProto(const google::protobuf::Message& msg, claidservice::Blob& targetBlob);
bool parseFromBlob(const claidservice::Blob& srcBlob, google::protobuf::Message& msg);

template<typename T>
static void setPayload(DataPackage& packet, T& data)
{
    claidservice::Blob& blob = *packet.mutable_payload();
    blobMsgFromProto(data, blob);
}

template<typename T> static void setIntVal(DataPackage& packet, const T& number)
{
    claidservice::IntVal val;
    val.set_val(number);
    setPayload(packet, val);
}

static void setStringVal(DataPackage& packet, const std::string& str)
{
    claidservice::StringVal val;
    val.set_val(str);
    setPayload(packet, val);
}


double getIntVal(DataPackage& packet);
std::string getStringVal(DataPackage& packet);

// Definitino of the modules and channels.
const std::string
    mod1 = "module_1",
    mod2 = "module_2",
    mod3 = "module_3",
    mod4 = "module_4",
    mod5 = "module_5",
    chan12 = "chan12",
    chan13Num = "chan13-num",
    chan13Str = "chan13-str",
    chan23 = "chan23",
    chan23Proto = "chan23Proto",
    chan12To45 = "chan12To45",
    modClass1 = "OneModuleClass",
    modClass2 = "TwoModuleClass",
    modClass3 = "ThreeModuleClass",
    modClass4 = "FourModuleClass",
    modClass5 = "FiveModuleClass";

google::protobuf::Value createValue(const std::string& str);

google::protobuf::Struct createStruct(const std::initializer_list<std::pair<const std::string, std::string>>& props);

const google::protobuf::Struct
    props_1 = createStruct({{"prop11", "val11"}, {"prop12", "val12"}}),
    props_2 = createStruct({{"prop21", "val21"}}),
    props_3 = createStruct({{"prop31", "val31"}}),
    props_4 = createStruct({{"prop41", "val41"}}),
    props_5 = createStruct({{"prop41", "val41"}});


template<typename T>
void addValToNumberArray(DataPackage& packet, const T& number)
{
    claidservice::NumberArray val;
    parseFromBlob(packet.payload(), val);
    val.add_val(number);
    setPayload(packet, val);
}
// define the packets for the different channels.
// Note: If multiple modules can send/receive on a channel we need a
// packet for each in/out combination.
const auto examplePayload = mkExamplePayload();
const std::shared_ptr<DataPackage>
    chan12Pkt = makePkt(chan12, mod1, mod2, [](auto& p) 
    { 
        claidservice::NumberArray val;
        val.add_val(99);
        setPayload(p, val);
    }),
    chan13NumPkt = makePkt(chan13Num, mod1, mod3, [](auto& p) 
    { 
        claidservice::IntVal val;
        val.set_val(42);
        setPayload(p, val);
    }),
    chan13StrPkt = makePkt(chan13Str, mod1, mod3, [](auto& p) 
    {
        claidservice::StringVal val;
        val.set_val("hitchhiker"); 
        setPayload(p, val);
    }),
    chan23Pkt = makePkt(chan23, mod2, mod3, [](auto& p) 
    { 
        setIntVal(p, 1042);
    }),
    chan23ProtoPkt = makePkt(chan23Proto, mod2, mod3, [](auto& p) { blobMsgFromProto(*examplePayload, *p.mutable_payload()); }),
    chan14Pkt = makePkt(chan12To45, mod1, mod4, [](auto& p) { setIntVal(p, 42); }),
    chan15Pkt = makePkt(chan12To45, mod1, mod5, [](auto& p) { setIntVal(p, 43); }),
    chan24Pkt = makePkt(chan12To45, mod2, mod4, [](auto& p) { setIntVal(p, 44); }),
    chan25Pkt = makePkt(chan12To45, mod2, mod5, [](auto& p) { setIntVal(p, 45); });

const std::vector<ModInfo> testModules = {
    ModInfo{mod1, modClass1, props_1},
    ModInfo{mod2, modClass2, props_2},
    ModInfo{mod3, modClass3, props_3},
    ModInfo{mod4, modClass4, props_4},
    ModInfo{mod5, modClass5, props_5}
};
const std::set<std::string> supportedModClasses = { modClass1, modClass2, modClass3, modClass4, modClass5 };

const std::vector<std::shared_ptr<DataPackage>> testChannels = {
    chan12Pkt, chan13NumPkt, chan13StrPkt, chan23Pkt, chan23ProtoPkt, chan14Pkt, chan15Pkt, chan24Pkt, chan25Pkt
};

const std::map<std::string, std::string> channelToConnectionMappings = {
    make_pair(chan12, chan12),
    make_pair(chan13Num, chan13Num),
    make_pair(chan13Str, chan13Str),
    make_pair(chan23, chan23),
    make_pair(chan23Proto, chan23Proto),
    make_pair(chan12To45, chan12To45),
};

} // end namespace claidtest

#endif


