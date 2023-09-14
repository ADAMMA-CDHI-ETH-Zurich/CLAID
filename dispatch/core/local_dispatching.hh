#ifndef LOCAL_DISPATCHING_HH_
#define LOCAL_DISPATCHING_HH_

#include <algorithm>
#include <memory>
#include <string>
#include <thread>

#include <grpc/grpc.h>
#include <grpcpp/server.h>

#include "dispatch/proto/claidservice.grpc.pb.h"
#include "dispatch/core/shared_queue.hh"
#include "dispatch/core/module_table.hh"

namespace claid {

class RuntimeDispatcher {
  public:
    explicit RuntimeDispatcher(SharedQueue<claidservice::DataPackage>& inQueue,
                               SharedQueue<claidservice::DataPackage>& outQueue,
                               const claid::ChannelMap& chanMap);
    void shutdownWriterThread();
    bool alreadyRunning();
    grpc::Status startWriterThread(grpc::ServerReaderWriter<claidservice::DataPackage, claidservice::DataPackage>* stream);
    grpc::Status processReading(grpc::ServerReaderWriter<claidservice::DataPackage, claidservice::DataPackage>* stream);
  private:
    void processWriting(grpc::ServerReaderWriter<claidservice::DataPackage, claidservice::DataPackage>* stream);
    void processPacket(claidservice::DataPackage& pkt, grpc::Status& status);
  private:
    SharedQueue<claidservice::DataPackage>& incomingQueue;
    SharedQueue<claidservice::DataPackage>& outgoingQueue;
    const claid::ChannelMap& chanMap;
    std::mutex wtMutex; // protects the write thread
    std::unique_ptr<std::thread> writeThread;

    friend class ServiceImpl;
}; // class RuntimeDispatcher

class ServiceImpl final : public claidservice::ClaidService::Service {
  public:
    explicit ServiceImpl(claid::ModuleTable& moduleTable);
    // void shutdown();
    // virtual ~ServiceImpl() { shutdown(); };
    virtual ~ServiceImpl() { };

    // Retrieve the list of modules to instantiate for a runtime
    grpc::Status GetModuleList(grpc::ServerContext* context,
        const claidservice::ModuleListRequest* req,
        claidservice::ModuleListResponse* resp) override;

    // Initialize modules for a runtime
    grpc::Status InitRuntime(grpc::ServerContext* context, const claidservice::InitRuntimeRequest* request,
        google::protobuf::Empty* response) override;


    grpc::Status SendReceivePackages(grpc::ServerContext* context,
        grpc::ServerReaderWriter<claidservice::DataPackage, claidservice::DataPackage>* stream) override;

  // private methods
  private:
    RuntimeDispatcher* addRuntimeDispatcher(claidservice::DataPackage& pkt, grpc::Status& status);
    void removeRuntimeDispatcher(RuntimeDispatcher& inst);

  // Data members
  private:
    claid::ModuleTable& moduleTable;
    std::mutex adMutex;    // protects activeDispatchers
    std::map<claidservice::Runtime, std::unique_ptr<RuntimeDispatcher>> activeDispatchers;
};     // class ServiceImpl

class DispatcherServer {
  public:
    DispatcherServer(const std::string& addr, claid::ModuleTable& modTable);
    virtual ~DispatcherServer() {};
    bool start();
    void shutdown();

  private:
    void buildAndStartServer();

  private:
    const std::string& addr;
    ServiceImpl serviceImpl;
    std::unique_ptr<grpc::Server> server;

};

// DispatcherClient connects to a local DispatcherServer over a
// unix domain socket and provides input and output queues.
class DispatcherClient {
  public:
    DispatcherClient(const std::string& socketPath,
        SharedQueue<claidservice::DataPackage>& inQueue,
        SharedQueue<claidservice::DataPackage>& outQueue,
        const std::set<std::string>& supportedModClasses);
    void shutdown();
    virtual ~DispatcherClient() { shutdown(); };
    std::unique_ptr<claidservice::ModuleListResponse> getModuleList();
    bool startRuntime(const claidservice::InitRuntimeRequest& req);
  private:
    void processReading();
    void processWriting();

  private:
    std::shared_ptr<grpc::Channel> grpcChannel;
    std::unique_ptr< claidservice::ClaidService::Stub> stub;
    SharedQueue<claidservice::DataPackage>& incomingQueue;
    SharedQueue<claidservice::DataPackage>& outgoingQueue;
    std::set<std::string> moduleClasses;
    std::shared_ptr<grpc::ClientContext> streamContext;
    std::shared_ptr<grpc::ClientReaderWriter<claidservice::DataPackage, claidservice::DataPackage>> stream;
    std::unique_ptr<std::thread> readThread;
    std::unique_ptr<std::thread> writeThread;
};

}  // namespace claid

#endif  // LOCAL_DISPATCHING_HH_
