#include "dispatch/core/module_table.hh"

using grpc::Status;

using namespace std;
using namespace claid;
using namespace claidservice;

void ChannelMap::setExpectedChannels(const set<ChannelKey>& expectedChannels) {
    for(auto it = expectedChannels.begin(); it != expectedChannels.end(); it++) {
        chanMap[*it] = shared_ptr<DataPackage>(nullptr);  
    }
}

Status ChannelMap::setChannelTypes(const string& moduleId, 
            const google::protobuf::Map<string, DataPackage>& channels) {

    if (moduleId.empty()) {
        return Status(grpc::INVALID_ARGUMENT, "Module id must not be empty");
    }

    // Broadly lock until the entire input is processed. This makes this 
    // code simpler and since this is only during runtime setup it's not the most 
    // performance critical section. 
    unique_lock<shared_mutex> lock(mapMutex);
    for(auto inputIt = channels.begin(); inputIt != channels.end(); inputIt++) {
        // Validate the input packets
        if (!validPacketType(inputIt->second)) {
            return Status(grpc::INVALID_ARGUMENT, "Invalid packet type for channel");
        }

        const string& src = inputIt->second.source_host_module();
        const string& tgt = inputIt->second.target_host_module();

        // At least source or target has to be the module in question.
        if ((moduleId != src) && (moduleId != tgt)) {
            return Status(grpc::INVALID_ARGUMENT, "Invalid source/target in channel definition");
        }

        auto key = ChannelKey(inputIt->first, src, tgt);
        auto found = chanMap.find(key);
        if (found == chanMap.end()) {
            return Status(grpc::INVALID_ARGUMENT, "Unexpectexd channel information");
        }

        // If the channel registration exists make sure it matchtes
        if (found->second) {
            if (!compPacketType(*found->second, inputIt->second)) {
                return Status(grpc::INVALID_ARGUMENT, "Previously registered channel type doesn't match!");
            }
        } else {
            chanMap[key] = make_shared<DataPackage>(inputIt->second);
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
        ref.has_control_val());
}

bool ChannelMap::isValid(const DataPackage& pkt) const {
    shared_lock<shared_mutex> lock(const_cast<shared_mutex&>(mapMutex));

    // look up the channel and make sure it's valid 
    auto key = ChannelKey(pkt.channel(), pkt.source_host_module(), pkt.target_host_module());
    auto it = chanMap.find(key);
    if (it == chanMap.end()) {
        return false; 
    }
    return compPacketType(*it->second, pkt);
}

