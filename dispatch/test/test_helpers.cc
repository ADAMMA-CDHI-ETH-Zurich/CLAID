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

#include "dispatch/test/test_helpers.hh"

// #include <chrono>
// #include <thread>

// #include <google/protobuf/text_format.h>
// #include "dispatch/proto/claidservice.grpc.pb.h"


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

shared_ptr<claidservice::DataPackage> claidtest::makePkt(const string& channel, const string& src, const string& tgt, setterFn setter) {
    auto ret = make_shared<claidservice::DataPackage>();
    *ret->mutable_channel() = channel;
    *ret->mutable_source_module() = src;
    *ret->mutable_target_module() = tgt;
    setter(*ret);
    return ret;
}

shared_ptr<claidservice::DataPackage> claidtest::makeChanPacket(const string& channel, const string& src,
        map<tuple<string, string, string>, list<shared_ptr<DataPackage>>>& receivePkts, const vector<shared_ptr<DataPackage>>& testChannels, setterFn setter) {
    auto ret = make_shared<claidservice::DataPackage>();
    *ret->mutable_channel() = channel;
    *ret->mutable_source_module() = src;
    setter(*ret);

    // Add the packets we should receive as a result.
    for(auto it : testChannels) {
        if ((it->channel() == channel) && (it->source_module() == src)) {
            auto pkg = make_shared<claidservice::DataPackage>(*ret);
            *pkg->mutable_target_module() = it->target_module();
            auto key = make_tuple(pkg->channel(), pkg->source_module(), pkg->target_module());
            receivePkts[key].push_back(pkg);
        }
    }
    return ret;
}

unique_ptr<ExamplePayload> claidtest::mkExamplePayload() {
    auto ret = make_unique<ExamplePayload>();
    ret->set_name("john doe");
    ret->set_value(99);
    return ret;
}

// Compare two protobuf and return the result.
string claidtest::cmpMsg(const Message& m1, const Message& m2) {
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

void claidtest::clearSrcTgt(DataPackage& pkt) {
    pkt.clear_source_module();
    pkt.clear_target_module();
}

void claidtest::blobMsgFromProto(const Message& msg, Blob& targetBlob) {
    targetBlob.set_codec(Codec::CODEC_PROTO);
    targetBlob.set_message_type(msg.GetDescriptor()->full_name());
    string payload;
    msg.SerializeToString(&payload);
    targetBlob.set_payload(std::move(payload));
}

bool claidtest::parseFromBlob(const Blob& srcBlob, Message& msg) {
    if ((srcBlob.codec() != Codec::CODEC_PROTO) ||
        (srcBlob.message_type() != msg.GetDescriptor()->full_name()) ||
        (srcBlob.message_type().empty()) ||
        (srcBlob.payload().empty())) {
        return false;
    }

    return msg.ParseFromString(srcBlob.payload());
}

double claidtest::getNumberVal(DataPackage& packet)
{
    claidservice::NumberVal val;
    parseFromBlob(packet.payload(), val);
    return val.val();
}


google::protobuf::Value claidtest::createValue(const std::string& str) {
    google::protobuf::Value value;
    value.set_string_value(str);
    return value;
}

google::protobuf::Struct claidtest::createStruct(const std::initializer_list<std::pair<const std::string, std::string>>& props) {
    google::protobuf::Struct result;
    for (const auto& prop : props) {
        (*result.mutable_fields())[prop.first] = createValue(prop.second);
    }
    return result;
}