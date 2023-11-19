#include "dispatch/core/module_table.hh"
#include "dispatch/test/test_helpers.hh"
#include "gtest/gtest.h"

using namespace std;
using namespace claid;
using namespace claidservice;
using namespace claidtest;

// using namespace examplemsg;
// using namespace std::chrono_literals;

// using google::protobuf::Message;
// using google::protobuf::TextFormat;
// using google::protobuf::util::MessageDifferencer;
// using google::protobuf::util::MessageDifferencer;
// using google::protobuf::io::StringOutputStream;

TEST(ModuleTableTestSuite, BasicModuleTableTest) {
    ModuleTable modTable;
    for(auto& m : testModules) {
        modTable.setNeededModule(m.modId, m.modClass, m.props);
    }

    map<string, vector<DataPackage>> pktsVecMap;
    for(auto pkt : testChannels) {
        modTable.setExpectedChannel(pkt->channel(), pkt->source_module(), pkt->target_module());

        // Add the channel type for source modules.
        auto cpPkt = *pkt;
        cpPkt.clear_target_module();
        pktsVecMap[cpPkt.source_module()].push_back(cpPkt);

        // Add the channel type for target modules.
        cpPkt = *pkt;
        cpPkt.clear_source_module();
        pktsVecMap[cpPkt.target_module()].push_back(cpPkt);
    }

    // Make sure a wrong channel type is rejected.

    // We expecte the table to not be ready because we have not set the channel types.
    ASSERT_FALSE(modTable.ready());

    // Set the channel types and see if we are ready.
    for(auto modIt : pktsVecMap) {
        google::protobuf::RepeatedPtrField<claidservice::DataPackage> chanTypes(modIt.second.begin(), modIt.second.end());
        modTable.setChannelTypes(modIt.first, chanTypes);
    }

    // Add a type
    google::protobuf::RepeatedPtrField<claidservice::DataPackage> chanTypes;
    *chanTypes.Add() = *chan14Pkt;
    auto status = modTable.setChannelTypes(mod1, chanTypes);
    cout << status.error_message() << endl;
    ASSERT_FALSE(status.ok());

    cout << "Module Table:\n" << modTable.toString() << "\n" << endl;

    ASSERT_TRUE(modTable.ready());

    SharedQueue<claidservice::DataPackage> outQueue;

    shared_ptr<DataPackage> cpPkt;
    cpPkt = make_shared<DataPackage>(*chan14Pkt);

    // Make sure when sending on this channel we get two output packets.
    auto entry = modTable.isValidChannel(*cpPkt);
    ASSERT_TRUE(entry != nullptr);
    modTable.addOutputPackets(*cpPkt, entry, outQueue);
    ASSERT_TRUE(outQueue.size() == 2);
}
