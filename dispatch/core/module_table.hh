#ifndef MODULE_TABLE_HH_
#define MODULE_TABLE_HH_

#include <memory>
#include <set>
#include <shared_mutex>
#include <string>

#include <grpc/grpc.h>

#include "dispatch/proto/claidservice.grpc.pb.h"
#include "dispatch/core/shared_queue.hh"

namespace claid {

// Channel key is tuple(channelId, source, target).
typedef std::tuple<std::string, std::string, std::string> ChannelKey;
inline ChannelKey make_chan_key(const std::string& chanId, const std::string& src, const std::string& tgt) {
  return std::make_tuple(chanId, src, tgt);
}

class ChannelMap {
  public:
    // Initializes the channel map with the expected set of channels. 
    void setExpectedChannels(const std::set<ChannelKey>& expectedChannels);

    void setChannel(const ChannelKey& channelKey);

    // Verifies that the given channels are expected and sets their 
    // data types. 
    grpc::Status setChannelTypes(const std::string& moduleId, 
            const google::protobuf::Map<std::string, claidservice::DataPackage>& channels);

    // Verifies that a given packet sends on defined channel 
    bool isValid(const claidservice::DataPackage& pkt) const;
  private:
    std::shared_mutex mapMutex;  // mutex for channelMap
    std::map<ChannelKey, std::shared_ptr<claidservice::DataPackage>> chanMap;
};

bool compPacketType(const claidservice::DataPackage& ref, const claidservice::DataPackage& val);
bool validPacketType(const claidservice::DataPackage& ref);

typedef std::map<claidservice::Runtime, std::shared_ptr<SharedQueue<claidservice::DataPackage>>> RuntimeQueueMap;

class ModuleTable {
  public:
    virtual ~ModuleTable() {}
    inline SharedQueue<claidservice::DataPackage>& inputQueue() {return fromModuleQueue;}
    void setModule(const std::string& moduleId, 
        const std::string& moduleClass, 
        const claidservice::Runtime runtime, 
        const std::map<std::string, std::string>& properties);
    void setChannel(const std::string& channelId, const std::string& source, const std::string& target);
    SharedQueue<claidservice::DataPackage>* lookupOutputQueue(const std::string& moduleId);

  private:
    SharedQueue<claidservice::DataPackage> fromModuleQueue;   
    std::map<std::string, claidservice::Runtime>  moduleRuntimeMap;  // map module ==> runtime
    std::map<std::string, std::map<std::string, std::string>> moduleProperties;  // properties for each module 
    std::map<claidservice::Runtime, std::unordered_set<std::string>> runtimeModuleMap;  // map runtime to set of modules 
    RuntimeQueueMap runtimeQueueMap; // map from runtime to outgoing queue 
    ChannelMap channelMap; // map from <channel_id, src, tgt> to DataPacket (= data type of channel )

  friend class ServiceImpl;
};

}  // namespace claid  

#endif  // MODULE_TABLE_H_
