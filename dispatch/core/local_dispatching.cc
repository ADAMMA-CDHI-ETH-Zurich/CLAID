#include "dispatch/core/local_dispatching.hh"

#include <grpcpp/server_builder.h>
#include <grpcpp/create_channel.h>

#include <google/protobuf/text_format.h>

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

void RuntimeDispatcher::shutdownWriterThread() {
    {  // only lock for checking the writer thread.
        lock_guard<mutex> lock(wtMutex);
        if (!writeThread) {
            return;
        }
    }

    // Cause the writer thread to terminate and wait for it.
    outgoingQueue.push_front(nullptr);
    writeThread->join();
    writeThread = nullptr;
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
    cout << "Done with writer thread of runtime dispatcher !" << endl;
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

string msgToStringx(const google::protobuf::Message& msg) {
    string buf;
    if (google::protobuf::TextFormat::PrintToString(msg, &buf)) {
        return buf;
    }

    return "Message not valid (partial content: " +
            msg.ShortDebugString();
}

void RuntimeDispatcher::processPacket(DataPackage& pkt, Status& status) {
    status = Status::OK;

    // Enqueue the data package and we are all done.
    if (!pkt.has_control_val()) {
        // filter the messages for consistency
        // check if src & dest are not in that
        if (!chanMap.isValid(pkt)) {
            cerr << "Received invalid data packet" << endl;
            cerr << msgToStringx(pkt) << "---------------------------" << endl;
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
    if (req->runtime() == Runtime::RUNTIME_UNSPECIFIED) {
        return Status(grpc::INVALID_ARGUMENT, "Invalid runtime value provided");
    }

    // Register the runtime
    for(auto it : req->supported_module_classes()) {
        // Look up the module class
        auto rt = moduleTable.moduleClassRuntimeMap[it];

        // The module class was already registered for this runtime
        // This is a no-op.
        if (rt == req->runtime()) {
            continue;
        }

        if (rt != Runtime::RUNTIME_UNSPECIFIED) {
            return Status(grpc::INVALID_ARGUMENT, "Attempted redefiniton of the runtime for a module class");
        }
        moduleTable.moduleClassRuntimeMap[it] = req->runtime();
    }

    // Iterate over
    for(auto it : moduleTable.moduleToClassMap) {
        auto desc = resp->add_descriptors();
        desc->set_module_id(it.first);
        desc->set_module_class(it.second);
        desc->mutable_properties()->insert(
            moduleTable.moduleProperties[it.first].begin(),
            moduleTable.moduleProperties[it.first].end());
    }
    return Status::OK;
}

Status ServiceImpl::InitRuntime(ServerContext* context, const InitRuntimeRequest* request, Empty* response) {
    // Make sure we have a valid runtime ID and there is matching queue for the runtime.
    Runtime rt = request->runtime();
    if (rt == Runtime::RUNTIME_UNSPECIFIED) {
        return Status(grpc::INVALID_ARGUMENT, "Runtime identifier missing");
    }

    for(auto modChanIt : request->modules()) {
        // find module --> moduleClass --> runtime
        auto& moduleId = modChanIt.module_id();

        map<string, string>::const_iterator modClassIt;
        if ((modClassIt = moduleTable.moduleToClassMap.find(moduleId))== moduleTable.moduleToClassMap.end()) {
            return Status(grpc::INVALID_ARGUMENT, "Unknown module id given");
        }

        auto classRt = moduleTable.moduleClassRuntimeMap[modClassIt->second];
        if (classRt != rt) {
            return Status(grpc::INVALID_ARGUMENT, "Runtime ids to not match");
        }
        moduleTable.moduleRuntimeMap[moduleId] = classRt;
        if (!moduleTable.runtimeQueueMap[classRt]) {
            moduleTable.runtimeQueueMap[classRt] = make_shared<SharedQueue<DataPackage>>();
        }

    // const unordered_set<string>& expectedMods = it->second;
    // auto& mods = request->modules();
    // for(auto it = mods.begin(); it != mods.end(); it++) {
    //     auto& moduleId = it->module_id();
    //     if (expectedMods.find(moduleId)==expectedMods.end()) {
    //         return Status(grpc::INVALID_ARGUMENT, "Unknown module id");
    //     }

        // Add the channels for this module
        Status status = moduleTable.channelMap.setChannelTypes(moduleId, modChanIt.channel_packets());
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
        return Status(grpc::CANCELLED, "Unable to read input package !");
    }

    // Add the runtime implemenation to the internal list of runtime dispatcher.
    RuntimeDispatcher* rtDispatcher = addRuntimeDispatcher(inPkt, status);
    if (!status.ok()) {
        return status;
    }

    cout << "runtime dispatcher created !" << endl;

    // TODO: Add cleanup function to clean up when we leave the scope.
    if (rtDispatcher->alreadyRunning()) {
        cout << "Writter thread was already running !" << endl;
        return Status(grpc::ALREADY_EXISTS, "Runtime dispatcher already exists");
    }

    // Confirm that we are getting ready to write.
    DataPackage outPkt = inPkt;
    stream->Write(outPkt);

    cout << "Starting writer thread !" << endl;

    rtDispatcher->startWriterThread(stream);
    status = rtDispatcher->processReading(stream);
    // rtDispatcher.shutdownWriter();

    return status;
}

RuntimeDispatcher* ServiceImpl::addRuntimeDispatcher(DataPackage& pkt, Status& status) {
    status = Status::OK;

    // Make sure we got a control package with a CTRL_RUNTIME_PING message.
    auto ctrlType = pkt.control_val().ctrl_type();
    auto runTime = pkt.control_val().runtime();
    cout << "Got package:" << ctrlType << "   :    " << runTime << endl;

    if (ctrlType != CtrlType::CTRL_RUNTIME_PING) {
        status = Status(grpc::INVALID_ARGUMENT, "Runtime init failed");
        return nullptr;
    }
    cout << "Runtime init confirmed " << endl;

    // check if the RuntimeDispatcher exits
    lock_guard<mutex> lock(adMutex);
    auto it = activeDispatchers.find(runTime);
    if (it != activeDispatchers.end()) {
        return it->second.get();
    }

    // Allocate a runtime Disptacher
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
        SharedQueue<DataPackage>& outQueue,
        const set<string>& supportedModClasses) :
        incomingQueue(inQueue), outgoingQueue(outQueue),
        moduleClasses(supportedModClasses) {
    // Set up the gRCP channel
    grpcChannel = grpc::CreateChannel(socketPath, grpc::InsecureChannelCredentials());
    stub = ClaidService::NewStub(grpcChannel);

}

void DispatcherClient::shutdown() {
    // Closing the outgoing queue will end the writer thread.
    outgoingQueue.close();
    if (writeThread) {
        writeThread->join();
        writeThread = nullptr;
    }

    if (readThread) {
        readThread->join();
        readThread = nullptr;
    }
}

unique_ptr<ModuleListResponse> DispatcherClient::getModuleList() {
    ClientContext context;
    ModuleListRequest req;
    req.set_runtime(Runtime::RUNTIME_CPP);
    for(auto it : moduleClasses) {
        req.add_supported_module_classes(it);
    }

    unique_ptr<ModuleListResponse> resp = make_unique<ModuleListResponse>();
    Status status = stub->GetModuleList(&context, req, resp.get());

    return (status.ok()) ? (move(resp)) : (unique_ptr<ModuleListResponse>(nullptr));
}

void makeControlRuntimePing(ControlPackage& pkt) {
    pkt.set_ctrl_type(CtrlType::CTRL_RUNTIME_PING);
    pkt.set_runtime(Runtime::RUNTIME_CPP);
}

bool DispatcherClient::startRuntime(const InitRuntimeRequest& req) {
    // Initialize the runtime
    {
        cout << "checkpoint 10" << endl;

        ClientContext context;
        cout << "checkpoint 11" << endl;
        Empty empty;
        cout << "checkpoint 12" << endl;
        Status status = stub->InitRuntime(&context,req, &empty);

        cout << "checkpoint 13" << endl;

        if (!status.ok()) {
            cerr << "couldn't init request: " << status.error_message() << endl;
            return false;
        }
        cout << "checkpoint 14" << endl;
    }

    // Send the ping package
    // if (true)
    {
        // Set timeout for API
        // std::chrono::system_clock::time_point deadline =
        //     std::chrono::system_clock::now() + std::chrono::seconds(10);
        // context.set_deadline(deadline);

        streamContext = make_shared<ClientContext>();
        stream = stub->SendReceivePackages(streamContext.get());

        cout << "checkpoint 15" << endl;

        claidservice::DataPackage pingReq;

        cout << "checkpoint 16" << endl;

        makeControlRuntimePing(*pingReq.mutable_control_val());

        cout << "checkpoint 17" << endl;

        if (!stream->Write(pingReq)) {
            cout << "Failed sending ping package to server !" << endl;
            return false;
        }

        cout << "Sent ping package to server !" << endl;

        // Wait for the valid response ping
        DataPackage pingResp;
        if (!stream->Read(&pingResp)) {
            cout << "Did not receive a ping package from server !" << endl;
            return false;
        }

        // TODO: @Stephan, shouldn't this here be pingResp ? 
        if (pingReq.control_val().ctrl_type() != CtrlType::CTRL_RUNTIME_PING) {
            return false;
        }

        // Start the threads to service the input/output queues.
        cout << "Starting i/o threads !" << endl;
        writeThread = make_unique<thread>([this]() { processWriting(); });
        readThread = make_unique<thread>([this]() { processReading(); });

        cout << "Checkpoint 199" << endl;
        return true;
    }
}

void DispatcherClient::processReading() {
    DataPackage dp;
    cout << "Before reading" << endl;
    while(stream->Read(&dp)) {
        cout << "Read Packet " << endl;
        incomingQueue.push_back(make_shared<DataPackage>(dp));
    }
    cout << "After reading" << endl;
}

void DispatcherClient::processWriting() {
    cout << "Before writing" << endl;

    while(true) {
        auto pkt = outgoingQueue.pop_front();
        cout << "Writing packet to " << pkt << endl;
        if (!pkt) {
            if (outgoingQueue.is_closed()) {
                break;
            }
        } else {
            if (!stream->Write(*pkt)) {
                cout << "Error writing " << endl;
                break;
            }
        }
    }
    cout << "After writing" << endl;
    stream->WritesDone();
}
