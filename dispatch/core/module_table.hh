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

// tuple<source_module, target_module, example package (specifies data type)>
typedef std::tuple<std::string, std::string, std::shared_ptr<claidservice::DataPackage>> ChannelInfo;

struct ModuleTableProperties {
  std::string userId;
  std::string deviceId;
};

struct ChannelEntry {
  std::map<std::string, bool> sources;
  std::map<std::string, bool> targets;
  claidservice::DataPackage::PayloadOneofCase payloadType;

  // Either adds or sets the given key in sources or targets.
  // 'isSrc' selects the target field. If the given key exists
  // it will be set to true, if it doesn't exist it will be added.
  // The function returns true if they key existed beforehand.
  bool addSet(const std::string& key, bool isSrc);
};

class ModuleTable {
  public:
    virtual ~ModuleTable() {}
    inline SharedQueue<claidservice::DataPackage>& inputQueue() {return fromModuleQueue;}
    SharedQueue<claidservice::DataPackage>* lookupOutputQueue(const std::string& moduleId);
    std::vector<std::shared_ptr<SharedQueue<claidservice::DataPackage>>> getRuntimeQueues();

    inline SharedQueue<claidservice::DataPackage>& controlPackagesQueue() {return receivedControlPackagesQueue;}

    void setProperties(const ModuleTableProperties& props);

    // Sets the information about the modules that have to be provided by
    // one of the connecting runtimes. This happens before any runtime connects.
    void setNeededModule(const std::string& moduleId,
        const std::string& moduleClass,
        const std::map<std::string, std::string>& properties);

    void setModuleChannelToConnectionMappings(const std::string& moduleId,
      const std::map<std::string, std::string>& inputChannelToConnectionMappping,
      const std::map<std::string, std::string>& outputChannelToConnectionMapping);

    // Sets a channel that will be defined and used by the connecting runtimes.
    // Usually this is information extracted from a config file. This happens before
    // any runtimes connect.
    void setExpectedChannel(const std::string& channelId, const std::string& source, const std::string& target);

    // Verifies that the given channels are expected and sets their data types.
    // This is called as runtimes connect and verifies that the channels provided by the
    // runtimes are correct.
    grpc::Status setChannelTypes(const std::string& moduleId,
            const google::protobuf::RepeatedPtrField<claidservice::DataPackage>& channels);

    bool ready() const;

    // Verifies that a given packet uses a previously verified channel and that the type
    // of the data is correct.
    // It returns the list modules that need to receive the given package.
    const ChannelEntry* isValidChannel(const claidservice::DataPackage& pkt) const;

    void addOutputPackets(const claidservice::DataPackage pkt,
                          const ChannelEntry* chanEntry,
                          SharedQueue<claidservice::DataPackage>& queue) const;

    void addRuntimeIfNotExists(const claidservice::Runtime& runtime);

    // Verifies that a given packet uses a previously verified channel and that the type
    // of the data is correct.
    std::shared_ptr<SharedQueue<claidservice::DataPackage>> getOutputQueueOfRuntime(const claidservice::Runtime& runtime);

    const std::string toString() const;

    // ONLY FOR TESTING REMOVE LATER
    void addModuleToRuntime(const std::string& Module, claidservice::Runtime runtime);

    void appendModuleAnnotations(const std::map<std::string, claidservice::ModuleAnnotation>& moduleAnnotations);
    const std::map<std::string, claidservice::ModuleAnnotation> getModuleAnnotations() const;
    bool getAnnotationForModule(const std::string& moduleClass, claidservice::ModuleAnnotation& annotation) const;

    void getRunningModules(std::vector<std::string>& moduleIDs) const;

    size_t getNumberOfRunningRuntimes();

    void clearLookupTables();

    bool lookupOutputConnectionForChannelOfModule(const std::string& sourceModule, const std::string& channelName, std::string& connectionName) const;

  private:
    void augmentFieldValues(claidservice::DataPackage& pkt) const;
    ChannelEntry* findChannel(const std::string& channelId);

  private:
    SharedQueue<claidservice::DataPackage> fromModuleQueue;

    // Queue where the LocalRouter will put received control packages.
    // This queue then can be processed by the middleware.
    // Without this queue, control packages would need to be handled
    // directly in the LocalRouter.
    SharedQueue<claidservice::DataPackage> receivedControlPackagesQueue;

    // Keeps track of mapping from the module instance to the name of that class that implements it.
    // map<module_id, module_class>
    std::map<std::string, std::string> moduleToClassMap; // maps module IDs to moduleClasses

    // Maps the channel names of a module to the corresponding connection names.
    // E.g., if the Module has a channel called "InputAudioData", the user might connect it to the Channel "AudioOutput" of another Module.
    // He could do so by mapping "InputAudioData" on a Connection "AudioStream" and then mapping "AudioOutput" to the same "AudioStream" connection.
    std::map<std::string, std::map<std::string, std::string>> moduleInputChannelsToConnectionMap;
    std::map<std::string, std::map<std::string, std::string>> moduleOutputChannelsToConnectionMap;

    // Keeps track of the properties of each module.
    // map<module_id, Map<property_key, property_value>>
    std::map<std::string, std::map<std::string, std::string>> moduleProperties;  // properties for each module

    // Populated during GetModuleList. Captures which module classes are provided by what runtime.
    std::map<std::string, claidservice::Runtime> moduleClassRuntimeMap; // maps moduleClasses to the runtime that provides the implementation

    // The output queues for each runtime. These persist even if the runtime
    // is restarted otherwise they should not be stored here but with the corresponding
    // runtime connection.
    // map<runtime, output_queue_for_runtime>
    RuntimeQueueMap runtimeQueueMap;

    // Keeps track of which module is statisfied by which runtime.
    // map<module_id, runtime>
    std::map<std::string, claidservice::Runtime>  moduleRuntimeMap;

    // Contains the properties that are shared across all modules managed by this module table,
    // e.g. user_id, device_id etc. These are used to populate the fields of client runtime implemenations.
    ModuleTableProperties props;

    // Mutex to protect the channel map.
    std::shared_mutex chanMapMutex;

    // TODO: rewrite doc

    // Maps from the channel name to the properties (type, direction, etc) of each
    // channel.
    // map<channel_id, map<src_module, list<DataPackage>>
    // Each DataPackage captures the channel, source, target, type. This is used for fast
    // lookup, because for each incoming packet we need to look up whether the
    // the packet is allowed on the given given channel and sender. For each sender we
    // dann have to replicate the package for all receivers (which might be more than one).
    //

    std::map<std::string, ChannelEntry> chanMap;

    std::map<std::string, claidservice::ModuleAnnotation> moduleAnnotations;

  friend class ServiceImpl;
};

}  // namespace claid

#endif  // MODULE_TABLE_H_
