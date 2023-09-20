#ifndef MODULE_TABLE_HH_
#define MODULE_TABLE_HH_

#include <memory>
// #include <set>
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
    void setChannel(const ChannelKey& channelKey);

    // Verifies that the given channels are expected and sets their
    // data types.
    grpc::Status setChannelTypes(const std::string& moduleId,
            const google::protobuf::RepeatedPtrField<claidservice::DataPackage>& channels);

    // Verifies that a given packet sends on defined channel
    bool isValid(const claidservice::DataPackage& pkt) const;

    const std::string toString() const;

    std::tuple<std::string, std::string, bool> lookupChannel(const std::string& channelId) const;

  private:
    std::shared_mutex mapMutex;  // mutex for channelMap
    std::map<ChannelKey, std::shared_ptr<claidservice::DataPackage>> chanMap;
    std::map<std::string, std::pair<std::string, std::string>> chanSrcTgtMap;
};

bool compPacketType(const claidservice::DataPackage& ref, const claidservice::DataPackage& val);
bool validPacketType(const claidservice::DataPackage& ref);

typedef std::map<claidservice::Runtime, std::shared_ptr<SharedQueue<claidservice::DataPackage>>> RuntimeQueueMap;

class ModuleTable {
  public:
    virtual ~ModuleTable() {}
    inline SharedQueue<claidservice::DataPackage>& inputQueue() {return fromModuleQueue;}
    SharedQueue<claidservice::DataPackage>* lookupOutputQueue(const std::string& moduleId);
    void setModule(const std::string& moduleId,
        const std::string& moduleClass,
        const std::map<std::string, std::string>& properties);
    void setChannel(const std::string& channelId, const std::string& source, const std::string& target);

    const std::string toString() const;

    // std::pair<std::string, std::string> lookupChannel(const string& chanId);

    // ONLY FOR TESTING REMOVE LATER
    void addModuleToRuntime(const std::string& Module, claidservice::Runtime runtime);

  private:
    SharedQueue<claidservice::DataPackage> fromModuleQueue;
    // These two maps capture the target configuration.
    std::map<std::string, std::string> moduleToClassMap; // maps module IDs to moduleClasses
    std::map<std::string, std::map<std::string, std::string>> moduleProperties;  // properties for each module

    // populated during ModuleList. Answers: which module classes are provided by what runtime.
    std::map<std::string, claidservice::Runtime> moduleClassRuntimeMap; // maps moduleClasses to the runtime that provides the implementation

    // Maps from module to runtime queu.
    RuntimeQueueMap runtimeQueueMap; // map from runtime to outgoing queue
    ChannelMap channelMap; // map from <channel_id, src, tgt> to DataPacket (= data type of channel )
    std::map<std::string, claidservice::Runtime>  moduleRuntimeMap;  // map module ==> runtime

  friend class ServiceImpl;
};

}  // namespace claid

#endif  // MODULE_TABLE_H_
