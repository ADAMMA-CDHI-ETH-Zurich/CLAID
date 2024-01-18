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
        // ((ref.source_module() == val.source_module()) ||
        // (ref.target_module() == val.target_module())) &&
        (ref.payload_oneof_case() == val.payload_oneof_case());
}

// Returns true if the packet from the client is valid.
// A packet is considered valid when
//    - the channel and payload are set
//    - the payload is not a control package
//    - the source or the target are set (exclusive or)
//
bool claid::validPacketType(const DataPackage& ref) {
    return !(ref.channel().empty() ||
        (ref.source_module().empty() && ref.target_module().empty()) ||
        (!ref.source_module().empty() && !ref.target_module().empty()) ||
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

std::vector<std::shared_ptr<SharedQueue<claidservice::DataPackage>>> ModuleTable::getRuntimeQueues()
{
    std::vector<std::shared_ptr<SharedQueue<claidservice::DataPackage>>> queues;
    for(auto& entry : runtimeQueueMap)
    {
        queues.push_back(entry.second);
    }
    return queues;
}


void ModuleTable::setProperties(const ModuleTableProperties& props) {
    this->props = props;
}

void ModuleTable::setNeededModule(const string& moduleId, const string& moduleClass,
        const map<string, string>& properties) {
    // TODO: verify no module is registered incorreclty multiple times
    moduleToClassMap[moduleId] = moduleClass;
    moduleProperties[moduleId] = properties;
}

void ModuleTable::setExpectedChannel(const string& channelId, const string& source, const string& target) {
    unique_lock<shared_mutex> lock(chanMapMutex);

    // TODO: Check whether the input doesn't conflict with
    // assumptions and previous additons of channels.
    chanMap[channelId].sources[source] = false;
    chanMap[channelId].targets[target] = false;
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
        // Validate the input packets, which guarantees that all required fields are set.
        if (!validPacketType(chanPkt)) {
            return Status(grpc::INVALID_ARGUMENT,
                "Invalid packet type for channel '" +
                chanPkt.channel() + "' : " + messageToString(chanPkt));
        }

        const string& channelId = chanPkt.channel();
        const string& src = chanPkt.source_module();
        const string& tgt = chanPkt.target_module();

        // At least source or target has to be the module in question, i.e., moduleId is either
        // subscriber or publisher for this channel.
        if ((moduleId != src) && (moduleId != tgt)) {
            return Status(grpc::INVALID_ARGUMENT, absl::StrCat(
                "Invalid source/target Module for Channel \"", channelId, "\".\n",
                "Module \"", moduleId, "\" has to be either publisher or subscriber of the Channel, but neither was set."));
        }


        // Find the channel and verify and set the type.
        auto entry = findChannel(channelId);
        if (!entry) {
            return Status(grpc::INVALID_ARGUMENT, absl::StrCat("Channel \"", channelId, "\" not known, did you specify it in the configuration file?"));
        }

        // If the type was already set and the type doesn't match we return an error.
        if ((entry->payloadType != DataPackage::PAYLOAD_ONEOF_NOT_SET) && (entry->payloadType != chanPkt.payload_oneof_case())) {
            return Status(grpc::INVALID_ARGUMENT, "Invalid packet type for channel '" +
                                                    chanPkt.channel() + "' : " + messageToString(chanPkt));
        }

        // Mark the source / target as matched by this module.
        bool isSource = (moduleId == src);
        const string& val = isSource ? src : tgt;
        if (!entry->addSet(val, isSource)) {
            return Status(grpc::INVALID_ARGUMENT, absl::StrCat(
                "Module \"", val, "\" ", isSource ? "published" : "subscribed", " channel \"", channelId, "\", ",
                "but the channel was not specified in the configuration file. Make sure to add the channel as ", 
                isSource ? "output channel" : "input channel", " of Module \"", val,  "\" in the configuration file."
            ));
        }
        Logger::logInfo("Setting channel %s %d", channelId.c_str(), chanPkt.payload_oneof_case());

        if(chanPkt.payload_oneof_case() == DataPackage::PayloadOneofCase::PAYLOAD_ONEOF_NOT_SET)
        {
            return Status(grpc::INVALID_ARGUMENT, absl::StrCat(
                "Failed to set channel \"", channelId, "\". Payload type is not set."
            ));
        }

        entry->payloadType = chanPkt.payload_oneof_case();
    }
    return Status::OK;
}

bool ModuleTable::ready() const {
    for(auto& it : chanMap) {
        for(auto& innerIt : it.second.sources) {
            if (!innerIt.second) {
                return false;
            }
        }

        for(auto& innerIt : it.second.targets) {
            if (!innerIt.second) {
                return false;
            }
        }
        if (it.second.payloadType == DataPackage::PAYLOAD_ONEOF_NOT_SET) {
            return false;
        }
    }
    return true;
}

const ChannelEntry* ModuleTable::isValidChannel(const DataPackage& pkt) const {

    shared_lock<shared_mutex> lock(const_cast<shared_mutex&>(chanMapMutex));

    auto entry = (const_cast<ModuleTable*>(this))->findChannel(pkt.channel());
    if (!entry) {
        return nullptr;
    }

    auto srcIt = entry->sources.find(pkt.source_module());
    if (srcIt == entry->sources.end()) {
        return nullptr;
    }

    if (entry->payloadType != pkt.payload_oneof_case()) {
        Logger::logError("Invalid package, payload type mismatch! Expected \"%d\" but got \"%d\"", entry->payloadType, pkt.payload_oneof_case());
        return nullptr;
    }


    return entry;
}

void ModuleTable::addOutputPackets(const claidservice::DataPackage pkt,
                          const ChannelEntry* chanEntry,
                          SharedQueue<claidservice::DataPackage>& queue) const {

    // Make a copy of the incoming packet and augment it.
    auto cpPkt = pkt;
    augmentFieldValues(cpPkt);
    for(auto& it : chanEntry->targets) {
        auto outPkt = make_shared<DataPackage>(cpPkt);
        outPkt->set_target_module(it.first);
        Logger::logInfo("Received package from %s", pkt.source_module().c_str());
        queue.push_back(outPkt);
    }
}

void ModuleTable::augmentFieldValues(claidservice::DataPackage& pkt) const {
    pkt.set_source_user_token(props.userId);
    pkt.set_device_id(props.deviceId);

    // TODO: Add more meta data fields here, like time stamps etc.
}

ChannelEntry* ModuleTable::findChannel(const string& channelId) {
    auto it = chanMap.find(channelId);
    if (it == chanMap.end()) {
        return nullptr;
    }
    return &it->second;
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
        out << "    \'" << it.first << "\'" << endl;
        out << "         Sources: ";
        for(auto& srcIt : it.second.sources) {
            out << srcIt.first << " : " << (srcIt.second ? "true" : "false") << "   ";
        }
        out << endl;

        out << "         Targets: ";
        for(auto& tgtIt : it.second.targets) {
            out << tgtIt.first << " : " << (tgtIt.second ? "true" : "false") << "  ";
        }
        out << endl;

        out << "         Type:" << it.second.payloadType << endl;
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

bool ChannelEntry::addSet(const string& key, bool isSrc) {
    map<string, bool>& field = (isSrc) ? sources : targets;
    auto it = field.find(key);
    if (it == field.end()) {
        field[key] = false;
        return false;
    }
    it->second = true;
    return true;
}


void ModuleTable::setModuleAnnotations(const std::map<std::string, claidservice::ModuleAnnotation>& moduleAnnotations)
{
    this->moduleAnnotations = moduleAnnotations;
}

const std::map<std::string, claidservice::ModuleAnnotation> ModuleTable::getModuleAnnotations() const
{
    return this->moduleAnnotations;
}

bool ModuleTable::getAnnotationForModule(const std::string& moduleClass, claidservice::ModuleAnnotation& annotation) const
{
    auto it = this->moduleAnnotations.find(moduleClass);
    if(it == this->moduleAnnotations.end())
    {
        return false;
    }

    annotation = it->second;
    return true;
}

void ModuleTable::getRunningModules(std::vector<std::string>& moduleIDs) const
{
    moduleIDs.clear();

    for(const auto& entry : this->moduleClassRuntimeMap)
    {
        moduleIDs.push_back(entry.first);
    }
}

