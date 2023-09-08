#include "dispatch/core/local_dispatching.hh"

#include <grpcpp/server_builder.h>
#include <grpcpp/create_channel.h>

// // For the server
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReaderWriter;
using grpc::Status;

using grpc::ClientContext;

using google::protobuf::Empty; 

using namespace std;
using namespace claid;
using namespace claidservice;

RuntimeDispatcher::RuntimeDispatcher(SharedQueue<DataPackage>& inQueue, 
                                     SharedQueue<DataPackage>& outQueue, 
                                     const ChannelMap& channelMap) :
        incomingQueue(inQueue), outgoingQueue(outQueue), chanMap(channelMap) {}

bool RuntimeDispatcher::alreadyRunning() {
    lock_guard<mutex> lock(wtMutex); 
    return bool(writeThread);  
}

Status RuntimeDispatcher::startWriterThread(ServerReaderWriter<DataPackage, DataPackage>* stream) {
    lock_guard<mutex> lock(wtMutex);
    if (writeThread) {
        return Status(grpc::INVALID_ARGUMENT, "Thread already running.");
    }
    writeThread = make_unique<thread>([this, stream]() {
        processWriting(stream); 
    });
    return Status::OK;
}

void RuntimeDispatcher::processWriting(ServerReaderWriter<DataPackage, DataPackage>* stream) {
     while(true) {
        auto pkt = outgoingQueue.pop_front(); 

        // If we got a null pointer we are done
        if (!pkt) {
            break; 
        } 

        if (!stream->Write(*pkt)) {
            outgoingQueue.push_front(pkt);
            break;
        }
    }
}

Status RuntimeDispatcher::processReading(ServerReaderWriter<DataPackage, DataPackage>* stream) {
    DataPackage inPkt; 
    Status status; 
    while(stream->Read(&inPkt)) {
        processPacket(inPkt, status);
        if (!status.ok()) {
            // TODO: remove once we have a cleanup function added. 
            return status; 
        }
    }
    return Status::OK; 
}

void RuntimeDispatcher::processPacket(DataPackage& pkt, Status& status) {
    status = Status::OK;
    
    // Enqueue the data package and we are all done. 
    if (!pkt.has_control_val()) {
        // filter the messages for consistency
        // check if src & dest are not in that
        if (!chanMap.isValid(pkt)) {
            cerr << "Received invalid data packet" << endl;
            return; 
        }
       incomingQueue.push_back(make_shared<DataPackage>(pkt));
       return; 
    }

    // Process the control values  
    auto ctrlType = pkt.control_val().ctrl_type();
    switch(ctrlType) {
        default: {
            status = Status(grpc::INVALID_ARGUMENT, "Invalid ctrl type");
            break;
        }
    }
}

ServiceImpl::ServiceImpl(ModuleTable& modTable)
    : moduleTable(modTable) {}

Status ServiceImpl::GetModuleList(ServerContext* context, 
        const ModuleListRequest* req, ModuleListResponse* resp) {
    // Iterate over the modules and copy them over. 
    for(auto it : moduleTable.moduleRuntimeMap) {
        if (it.second == req->runtime()) {
            auto desc = resp->add_descriptors();
            desc->set_module_id(it.first);

            // google::protobuf::Map<std::string, std::string> propCopy(
            //     moduleTable.moduleProperties.begin(), 
            //     moduleTable.moduleProperties.end());

            desc->mutable_properties()->insert(
                moduleTable.moduleProperties[it.first].begin(), 
                moduleTable.moduleProperties[it.first].end()); 
        }
    }

    return Status::OK;
}

Status ServiceImpl::InitRuntime(ServerContext* context, const InitRuntimeRequest* request, Empty* response) {
    // Make sure we have a valid runtime ID and there is matching queue for the runtime. 
    Runtime rt = request->runtime(); 
    if (rt == Runtime::RUNTIME_UNSPECIFIED) {
        return Status(grpc::INVALID_ARGUMENT, "Runtime identifier missing");
    }

    map<Runtime, unordered_set<string>>::const_iterator it;
    if ((it = moduleTable.runtimeModuleMap.find(rt))== moduleTable.runtimeModuleMap.end()) {
        return Status(grpc::INVALID_ARGUMENT, "Unknown runtime identifier");
    }

    const unordered_set<string>& expectedMods = it->second;
    auto& mods = request->modules(); 
    for(auto it = mods.begin(); it != mods.end(); it++) {
        auto& moduleId = it->module_id();
        if (expectedMods.find(moduleId)==expectedMods.end()) {
            return Status(grpc::INVALID_ARGUMENT, "Unknown module id");
        }

        // Add the channels for this module 
        Status status = moduleTable.channelMap.setChannelTypes(moduleId, it->channels());
        if (!status.ok()) {
            return status; 
        }
    }
 
    return Status::OK;
}

Status ServiceImpl::SendReceivePackages(ServerContext* context,
        ServerReaderWriter<DataPackage, DataPackage>* stream) {
    Status status;

    // Read the first packet 
    DataPackage inPkt; 
    if (!stream->Read(&inPkt)) {
        return Status::OK; 
    }

    // Add the runtime implemenation to the internal list of runtime dispatcher.
    RuntimeDispatcher* rtDispatcher = addRuntimeDispatcher(inPkt, status);
    if (!status.ok()) {
        return status; 
    }

    // TODO: Add cleanup function to clean up when we leave the scope. 
    if (rtDispatcher->alreadyRunning()) {
        return Status(grpc::ALREADY_EXISTS, "Runtime dispatcher already exists"); 
    }

    rtDispatcher->startWriterThread(stream);
    return rtDispatcher->processReading(stream);
}

RuntimeDispatcher* ServiceImpl::addRuntimeDispatcher(DataPackage& pkt, Status& status) {
    // Make sure we got a control package with a CTRL_RUNTIME_PING message.  
    auto ctrlType = pkt.control_val().ctrl_type(); 
    auto runTime = pkt.control_val().runtime(); 

    if (ctrlType != CtrlType::CTRL_RUNTIME_PING) {
        status = Status(grpc::INVALID_ARGUMENT, "Runtime init failed");
        return nullptr; 
    }

    // check if the RuntimeDispatcher exits 
    lock_guard<mutex> lock(adMutex);
    auto it = activeDispatchers.find(runTime);
    if (it != activeDispatchers.end()) {
        return it->second.get(); 
    }

    // Allocate
    shared_ptr<SharedQueue<DataPackage>> rtq = moduleTable.runtimeQueueMap[runTime];  
    auto ret = new RuntimeDispatcher(moduleTable.fromModuleQueue, *rtq, moduleTable.channelMap);
    activeDispatchers[runTime] = unique_ptr<RuntimeDispatcher>(ret); 
    return ret;
}

void ServiceImpl::removeRuntimeDispatcher(RuntimeDispatcher& inst) {
    // TODO
}

DispatcherServer::DispatcherServer(const string& addr, ModuleTable& modTable)
    : addr(addr), serviceImpl(modTable) {}

bool DispatcherServer::start() {
    buildAndStartServer(); 
    if (server) {
        std::cout << "Server listening on " << addr << std::endl;
    }
    return bool(server);
}

void DispatcherServer::shutdown() {
    server->Shutdown();
    server->Wait();
}

void DispatcherServer::buildAndStartServer() {
  ServerBuilder builder;
  builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
  builder.RegisterService(&serviceImpl);
  server = builder.BuildAndStart();
}

DispatcherClient::DispatcherClient(const string& socketPath,
        SharedQueue<DataPackage>& inQueue, 
        SharedQueue<DataPackage>& outQueue) :
        incomingQueue(inQueue), outgoingQueue(outQueue) {
    // Set up the gRCP channel 
    grpcChannel = grpc::CreateChannel(socketPath, grpc::InsecureChannelCredentials());
    stub = ClaidService::NewStub(grpcChannel);
}

unique_ptr<ModuleListResponse> DispatcherClient::getModuleList() {
    ClientContext context;
    ModuleListRequest req; 
    req.set_runtime(RUNTIME_CPP);

    unique_ptr<ModuleListResponse> resp = make_unique<ModuleListResponse>();
    Status status = stub->GetModuleList(&context, req, resp.get());

    return (status.ok()) ? (move(resp)) : (unique_ptr<ModuleListResponse>(nullptr));
}

claidservice::ControlPackage makeControlRuntimePing() { 
    claidservice::ControlPackage ret; 
    ret.set_runtime(Runtime::RUNTIME_CPP);
    return ret; 
}

bool DispatcherClient::startRuntime(const InitRuntimeRequest& req) {
    // Initialize the runtime 
    ClientContext context;
    Empty empty;
    Status status = stub->InitRuntime(&context,req, &empty);

    if (!status.ok()) {
        cout << "couldn't init request" << endl; 
        return false; 
    }

    // Send the ping package package
    stream = stub->SendReceivePackages(&context);
    claidservice::ControlPackage cPkg;
    DataPackage pingReq;
    *pingReq.mutable_control_val() = makeControlRuntimePing();
    if (!stream->Write(pingReq)) {
        return false; 
    }

    // Wait for the valid response ping 
    DataPackage pingResp;
    if (!stream->Read(&pingResp)) {
        return false; 
    }
    if (pingReq.control_val().ctrl_type() != CtrlType::CTRL_RUNTIME_PING) {
        return false; 
    }

    // Start the threads to service the input/output queues. 
    writeThread = make_unique<thread>([this]() { processWriting(); });
    readThread = make_unique<thread>([this]() { processReading(); });
    return true;
}

void DispatcherClient::processReading() {
    DataPackage dp; 
    while(stream->Read(&dp)) {
        incomingQueue.push_back(make_shared<DataPackage>(dp)); 
    }
}

void DispatcherClient::processWriting() {
    while(true) {
        auto pkt = outgoingQueue.pop_front();
        if (!stream->Write(*pkt)) {
            cout << "Error writing " << endl; 
            break; 
        }
    }
}
