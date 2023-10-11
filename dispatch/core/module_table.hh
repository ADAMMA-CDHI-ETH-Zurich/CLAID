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

bool compPacketType(const claidservice::DataPackage& ref, const claidservice::DataPackage& val);
bool validPacketType(const claidservice::DataPackage& ref);

typedef std::map<claidservice::Runtime, std::shared_ptr<SharedQueue<claidservice::DataPackage>>> RuntimeQueueMap;
typedef std::tuple<std::string, std::string, std::shared_ptr<claidservice::DataPackage>> ChannelInfo;

struct ModuleTableProperties {
  std::string userId;
  std::string deviceId;
};

class ModuleTable {
  public:
    virtual ~ModuleTable() {}
    inline SharedQueue<claidservice::DataPackage>& inputQueue() {return fromModuleQueue;}
    SharedQueue<claidservice::DataPackage>* lookupOutputQueue(const std::string& moduleId);
    void setProperties(const ModuleTableProperties& props);
    void setModule(const std::string& moduleId,
        const std::string& moduleClass,
        const std::map<std::string, std::string>& properties);
    void setChannel(const std::string& channelId, const std::string& source, const std::string& target);

    // Verifies that the given channels are expected and sets their
    // data types.
    grpc::Status setChannelTypes(const std::string& moduleId,
            const google::protobuf::RepeatedPtrField<claidservice::DataPackage>& channels);

    // Verifies that a given packet sends on defined channel
    bool isValidChannel(const claidservice::DataPackage& pkt, ChannelInfo& chanInfo) const;

    bool lookupChannel(const std::string& channelId, ChannelInfo& chanInfo) const;

    void augmentFieldValues(claidservice::DataPackage& pkt, const ChannelInfo& chanInfo) const;

    void addRuntimeIfNotExists(const claidservice::Runtime& runtime);

    std::shared_ptr<SharedQueue<claidservice::DataPackage>> getOutputQueueOfRuntime(const claidservice::Runtime& runtime);

    const std::string toString() const;

    // ONLY FOR TESTING REMOVE LATER
    void addModuleToRuntime(const std::string& Module, claidservice::Runtime runtime);

  private:
    bool containsChan(const std::string& chanId, ChannelInfo& entry) const;

  private:
    SharedQueue<claidservice::DataPackage> fromModuleQueue;

    // // These two maps capture the target configuration.
    std::map<std::string, std::string> moduleToClassMap; // maps module IDs to moduleClasses

    // SharedQueue<claidservice::DataPackage>& fromModuleQueue;
    std::map<std::string, std::map<std::string, std::string>> moduleProperties;  // properties for each module

    // // populated during ModuleList. Answers: which module classes are provided by what runtime.
    std::map<std::string, claidservice::Runtime> moduleClassRuntimeMap; // maps moduleClasses to the runtime that provides the implementation

    // // runtimeQueueMap from module to runtime queue.
    RuntimeQueueMap runtimeQueueMap; // map from runtime to outgoing queue

    std::map<std::string, claidservice::Runtime>  moduleRuntimeMap;  // map module ==> runtime
    ModuleTableProperties props;

    // // protect and store the channels and their types.
    std::shared_mutex chanMapMutex;  // mutex for chanMap
    std::map<std::string, ChannelInfo> chanMap;

  friend class ServiceImpl;
};

}  // namespace claid

#endif  // MODULE_TABLE_H_
