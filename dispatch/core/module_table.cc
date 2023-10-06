#include "dispatch/core/module_table.hh"
#include "dispatch/core/proto_util.hh"

// #include <google/protobuf/text_format.h>

using grpc::Status;

using namespace std;
using namespace claid;
using namespace claidservice;

// compPacketType compares whether the 'val' packet matches the
// channel and payload in the 'ref' packet.
bool claid::compPacketType(const DataPackage& ref, const DataPackage& val) {
    return (ref.channel() == val.channel()) &&
        // ((ref.source_host_module() == val.source_host_module()) ||
        // (ref.target_host_module() == val.target_host_module())) &&
        (ref.payload_oneof_case() == val.payload_oneof_case());
}

// Returns true if the packet from the client is valid.
// A packet is considered valid when
//    - the channel and payload are set
//    - the payload is not a control package
//    - the source or the target are set (depending whether it's an
//      input or output channel)
//
bool claid::validPacketType(const DataPackage& ref) {
    return !(ref.channel().empty() ||
        (ref.source_host_module().empty() && ref.target_host_module().empty()) ||
        (ref.payload_oneof_case() == DataPackage::PayloadOneofCase::PAYLOAD_ONEOF_NOT_SET) ||
        ref.has_control_val());
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

void ModuleTable::setProperties(const ModuleTableProperties& props) {
    this->props = props;
}

void ModuleTable::setModule(const string& moduleId, const string& moduleClass,
        const map<string, string>& properties) {
    // TODO: verify no module is registered incorreclty multiple times
    moduleToClassMap[moduleId] = moduleClass;
    moduleProperties[moduleId] = properties;
}

void ModuleTable::setChannel(const string& channelId, const string& source, const string& target) {
    unique_lock<shared_mutex> lock(chanMapMutex);
    chanMap[channelId] = make_tuple(source, target, nullptr);
}

Status ModuleTable::setChannelTypes(const string& moduleId,
            const google::protobuf::RepeatedPtrField<DataPackage>& channels) {
    if (moduleId.empty()) {
        return Status(grpc::INVALID_ARGUMENT, "Module id must not be empty");
    }

    // Broadly lock until the entire input is processed. This makes this
    // code simpler and since this is only during runtime setup it's not the most
    // performance critical section.
    unique_lock<shared_mutex> lock(chanMapMutex);
    for(auto& chanPkt : channels) {
        // Validate the input packets
        if (!validPacketType(chanPkt)) {
            return Status(grpc::INVALID_ARGUMENT,
                "Invalid packet type for channel '" +
                chanPkt.channel() + "' : " + messageToString(chanPkt));
        }

        // Make sure the source or target is set.
        const string& src = chanPkt.source_host_module();
        const string& tgt = chanPkt.target_host_module();

        // At least source or target has to be the module in question.
        if ((moduleId != src) && (moduleId != tgt)) {
            return Status(grpc::INVALID_ARGUMENT, "Invalid source/target in channel definition");
        }

        tuple<string, string, shared_ptr<DataPackage>> temp;
        if (!containsChan(chanPkt.channel(), temp)) {
            return Status(grpc::INVALID_ARGUMENT, "Channel '" + chanPkt.channel() + "' undefined.");
        }

        // If the channel registration exists make sure it matchtes
        if (get<2>(temp)) {
            if (!compPacketType(*get<2>(temp), chanPkt)) {
                return Status(grpc::INVALID_ARGUMENT, "Previously registered channel type doesn't match!");
            }
        } else {
            get<2>(chanMap[chanPkt.channel()]) = make_shared<DataPackage>(chanPkt);
        }
    }
    return Status::OK;
}

bool ModuleTable::isValidChannel(const DataPackage& pkt, ChannelInfo& chanInfo) const {
    shared_lock<shared_mutex> lock(const_cast<shared_mutex&>(chanMapMutex));

    if (!containsChan(pkt.channel(), chanInfo)) {
        return false;
    }
    return compPacketType(*get<2>(chanInfo), pkt);
}

bool ModuleTable::lookupChannel(const string& chanId, ChannelInfo& chanInfo) const {
    shared_lock<shared_mutex> lock(const_cast<shared_mutex&>(chanMapMutex));
    return containsChan(chanId, chanInfo);
}

void ModuleTable::augmentFieldValues(claidservice::DataPackage& pkt, const ChannelInfo& chanInfo) const {
    pkt.set_source_user_token(props.userId);
    pkt.set_device_id(props.deviceId);
    pkt.set_source_host_module(get<0>(chanInfo));
    pkt.set_target_host_module(get<1>(chanInfo));

    // TODO: Add more meta data fields here, like time stamps etc.
}

bool ModuleTable::containsChan(const string& chanId, tuple<string,string, shared_ptr<DataPackage>>& entry) const {
    auto it = chanMap.find(chanId);
    if (it == chanMap.end()) {
        return false;
    }
    entry = it->second;
    return true;
}

const string ModuleTable::toString() const {
    std::ostringstream out;
    out << "Modules: " << moduleToClassMap.size() << endl;
    for(auto& it : moduleToClassMap) {
        out << "    " << it.first << " ==> " << it.second << endl;
        out << "        Properties: " << endl;
        auto props = moduleProperties.find(it.first);
        if (props != moduleProperties.end()) {
            for(auto& propIt : props->second) {
                out << "            " << propIt.first << "=" << propIt.second << endl;
            }
        }
    }
    out << "Channels:" << endl;

    shared_lock<shared_mutex> lock(const_cast<shared_mutex&>(chanMapMutex));
    for(auto& it : chanMap) {
        out << "    ";
        out << "<" << it.first << ", " << get<0>(it.second) << ", " << get<1>(it.second) << "> : ";
        auto pkg = get<2>(it.second);
        if (pkg) {
            out << pkg->source_host_module() << " --> " << pkg->target_host_module();
        } else {
            out << "<none>";
        }
        out << endl;
    }

    out << "Chan Table: " << endl;
    for(auto it : chanMap) {
        out << "   " << it.first << " : "
            << get<0>(it.second) << " | "
            << get<1>(it.second) << " | "
            << (get<2>(it.second) ? (messageToString(*get<2>(it.second))) : string("none")) << endl << endl;
    }
    return out.str();
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