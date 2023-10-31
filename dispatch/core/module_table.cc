#include "dispatch/core/module_table.hh"
#include "dispatch/core/proto_util.hh"
#include "dispatch/core/Logger/Logger.hh"
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

        const string& channelId = chanPkt.channel();
        // Make sure the source or target is set.
        const string& src = chanPkt.source_host_module();
        const string& tgt = chanPkt.target_host_module();

        // At least source or target has to be the module in question, i.e., moduleId is either 
        // subscriber or publisher for this channel.
        if ((moduleId != src) && (moduleId != tgt)) {
            return Status(grpc::INVALID_ARGUMENT, absl::StrCat(
                "Invalid source/target Module for Channel \"", channelId, "\".\n",
                "Module \"", moduleId, "\" has to be either publisher or subscriber of the Channel, but neither was set."));
        }

        tuple<string, string, shared_ptr<DataPackage>> temp;
        // If the channel exists, the channel information will be assigned to temp.
        if (!containsChan(chanPkt.channel(), temp)) {
            return Status(grpc::INVALID_ARGUMENT, absl::StrCat(
                "Module \"", moduleId, "\" published or subscribed Channel \"", channelId, "\", ",
                "which was not specified in the configuration file for the Module."
            ));
        }

        // If the channel registration exists make sure it matchtes (i.e., data type is correct).
        if (get<2>(temp)) {
            if (!compPacketType(*get<2>(temp), chanPkt)) {
                const std::string previousPayloadType = dataPackagePayloadCaseToString(*std::get<2>(temp));
                const std::string newPayloadType = dataPackagePayloadCaseToString(chanPkt);
                return Status(grpc::INVALID_ARGUMENT, absl::StrCat("Mismatching data type for channel \"", channelId, "\".\n",
                "Channel was previously registered with payload type \"", previousPayloadType, "\", but Module \"", moduleId, "\""
                "tried to publish/subscribe with payload type \"", newPayloadType, "\"."));
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
    claid::Logger::logInfo("Contains chan %s\n", chanId.c_str());
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

void ModuleTable::addRuntimeIfNotExists(const claidservice::Runtime& runtime)
{
    auto it = runtimeQueueMap.find(runtime);
    if(it == runtimeQueueMap.end())
    {
        runtimeQueueMap.insert(make_pair(runtime, make_shared<SharedQueue<DataPackage>>()));
    }
}

std::shared_ptr<SharedQueue<claidservice::DataPackage>> ModuleTable::getOutputQueueOfRuntime(const claidservice::Runtime& runtime)
{
    auto it = runtimeQueueMap.find(runtime);
    if(it == runtimeQueueMap.end())
    {
        return nullptr;
    } 
    return it->second;
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