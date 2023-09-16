#include "dispatch/core/module_table.hh"

#include <google/protobuf/text_format.h>

using grpc::Status;

using namespace std;
using namespace claid;
using namespace claidservice;

string msgToStringy(const google::protobuf::Message& msg) {
    string buf;
    if (google::protobuf::TextFormat::PrintToString(msg, &buf)) {
        return buf;
    }

    return "Message not valid (partial content: " +
            msg.ShortDebugString();
}


void ChannelMap::setExpectedChannels(const set<ChannelKey>& expectedChannels) {
    for(auto it = expectedChannels.begin(); it != expectedChannels.end(); it++) {
        chanMap[*it] = shared_ptr<DataPackage>(nullptr);
    }
}

void ChannelMap::setChannel(const ChannelKey& channelKey) {
    chanMap[channelKey] = nullptr;
}

Status ChannelMap::setChannelTypes(const string& moduleId,
            const google::protobuf::RepeatedPtrField<DataPackage>& channels) {

            // const google::protobuf::Map<string, DataPackage>& channels) {

    if (moduleId.empty()) {
        return Status(grpc::INVALID_ARGUMENT, "Module id must not be empty");
    }

    // Broadly lock until the entire input is processed. This makes this
    // code simpler and since this is only during runtime setup it's not the most
    // performance critical section.
    unique_lock<shared_mutex> lock(mapMutex);
    for(auto& chanPkt : channels) {
        cout << "Init Channel packet -----> " << endl
            << msgToStringy(chanPkt) << endl
            << "----------------------------" << endl;

        // Validate the input packets
        if (!validPacketType(chanPkt)) {
            return Status(grpc::INVALID_ARGUMENT, "Invalid packet type for channel");
        }

        const string& src = chanPkt.source_host_module();
        const string& tgt = chanPkt.target_host_module();

        // At least source or target has to be the module in question.
        if ((moduleId != src) && (moduleId != tgt)) {
            return Status(grpc::INVALID_ARGUMENT, "Invalid source/target in channel definition");
        }

        auto key = ChannelKey(chanPkt.channel(), src, tgt);
        auto found = chanMap.find(key);
        if (found == chanMap.end()) {
            return Status(grpc::INVALID_ARGUMENT, "Unexpectexd channel information");
        }

        // If the channel registration exists make sure it matchtes
        if (found->second) {
            if (!compPacketType(*found->second, chanPkt)) {
                return Status(grpc::INVALID_ARGUMENT, "Previously registered channel type doesn't match!");
            }
        } else {
            chanMap[key] = make_shared<DataPackage>(chanPkt);
        }
    }
    return Status::OK;
}

bool claid::compPacketType(const DataPackage& ref, const DataPackage& val) {
    return (ref.channel() == val.channel()) &&
        (ref.source_host_module() == val.source_host_module()) &&
        (ref.target_host_module() == val.target_host_module()) &&
        (ref.payload_oneof_case() == val.payload_oneof_case());
}

bool claid::validPacketType(const DataPackage& ref) {
    return !(ref.channel().empty() ||
        ref.source_host_module().empty() ||
        ref.target_host_module().empty() ||
        (ref.payload_oneof_case() == DataPackage::PayloadOneofCase::PAYLOAD_ONEOF_NOT_SET) ||
        ref.has_control_val());
}

bool ChannelMap::isValid(const DataPackage& pkt) const {
    shared_lock<shared_mutex> lock(const_cast<shared_mutex&>(mapMutex));

    // look up the channel and make sure it's valid
    auto key = ChannelKey(pkt.channel(), pkt.source_host_module(), pkt.target_host_module());

    cout << "Channel key: " << get<0>(key) << ":" << get<1>(key) << ":" << get<2>(key) << endl;

    auto it = chanMap.find(key);
    if (it == chanMap.end()) {
        cout << "Channel key not found" << endl;
        return false;
    }
    cout << "Before comp " << endl <<
        msgToStringy(*it->second) << endl
        << "---------------------------------------" << endl
        << msgToStringy(pkt) << endl
        << "------------------" << endl;
    return compPacketType(*it->second, pkt);
}

void ModuleTable::setModule(const string& moduleId, const string& moduleClass,
        const map<string, string>& properties) {
    // TODO: verify no module is registered incorreclty multiple times
    moduleToClassMap[moduleId] = moduleClass;
    moduleProperties[moduleId] = properties;

    // auto runtime = Runtime::RUNTIME_CPP;
    // moduleRuntimeMap[moduleId] = runtime;
    // moduleProperties[moduleId] = properties;
    // runtimeModuleMap[runtime].insert(moduleId);
    // if (!runtimeQueueMap[runtime]) {
    //     runtimeQueueMap[runtime] = make_shared<SharedQueue<DataPackage>>();
    // }
}

void ModuleTable::setChannel(const string& channelId, const string& source, const string& target) {
    channelMap.setChannel(make_chan_key(channelId, source, target));
}

SharedQueue<DataPackage>* ModuleTable::lookupOutputQueue(const string& moduleId) {
    auto rt = moduleRuntimeMap[moduleId];
    if (rt != Runtime::RUNTIME_UNSPECIFIED) {
        auto outQueue = runtimeQueueMap[rt];
        if (outQueue) {
            return outQueue.get();
        }
    }
    return nullptr;
}

// ONLY FOR TESTING REMOVE LATER
void ModuleTable::addModuleToRuntime(const std::string& moduleID, claidservice::Runtime runtime)
{
    moduleRuntimeMap[moduleID] = runtime;
    auto outQueue = runtimeQueueMap[runtime];
    if(!outQueue)
    {
        runtimeQueueMap[runtime] = make_shared<SharedQueue<DataPackage>>();
    }
}