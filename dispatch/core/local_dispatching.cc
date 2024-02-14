#include "dispatch/core/local_dispatching.hh"
#include "dispatch/core/Logger/Logger.hh"
#include "dispatch/core/proto_util.hh"

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

// Validate ControlPackage with runtime ping.
static bool validCtrlRuntimePingPkt(const DataPackage& pkt) {
    return (pkt.control_val().ctrl_type() == CTRL_RUNTIME_PING) &&
            (pkt.control_val().runtime() != RUNTIME_UNSPECIFIED);
}

RuntimeDispatcher::RuntimeDispatcher(SharedQueue<DataPackage>& inQueue,
                                     SharedQueue<DataPackage>& outQueue,
                                     const ModuleTable& modTable) :
        incomingQueue(inQueue), outgoingQueue(outQueue), moduleTable(modTable)
{
    Logger::logInfo("constr incoming queue ptr: %lu", &incomingQueue);
    Logger::logInfo("const outgoing queue ptr: %lu", &outgoingQueue);

}

bool RuntimeDispatcher::alreadyRunning() {
    lock_guard<mutex> lock(wtMutex);
    return bool(writeThread);
}

Status RuntimeDispatcher::startWriterThread(ServerReaderWriter<DataPackage, DataPackage>* stream) {
    Logger::logInfo("Start writer thread 1");
    Logger::logInfo("Start writer thread 2");
    this->running = true;
    if (writeThread) {
        return Status(grpc::INVALID_ARGUMENT, "Thread already running.");
    }
    Logger::logInfo("Start writer thread 3");
    writeThread = make_unique<thread>([this, stream]() {
        processWriting(stream);
    });
    Logger::logInfo("Start writer thread 4");

    return Status::OK;
}

void RuntimeDispatcher::shutdownWriterThread() {
    {  // only lock for checking the writer thread.
        lock_guard<mutex> lock(wtMutex);
        if (!writeThread) {
            return;
        }
    }
    this->running = false;
    // Cause the writer thread to terminate and wait for it.
    outgoingQueue.interruptOnce();
    Logger::logInfo("Waiting for writer thread to be done.");
    writeThread->join();
    writeThread = nullptr;
    Logger::logInfo("Writer thread is finished.");
}

void RuntimeDispatcher::processWriting(ServerReaderWriter<DataPackage, DataPackage>* stream) {
     while(this->running) {

        Logger::logInfo("RuntimeDispatcher waiting");
        auto pkt = outgoingQueue.interruptable_pop_front();
        Logger::logInfo("RuntimeDispatcher::processWriting got package");
        // Might be received due to spurious wake up or queue closed
        if (!pkt) {
            if(outgoingQueue.is_closed())
            {
                break;
            }
            else
            {
                // Spurious wakeup
                continue;
            }
        }
        Logger::logInfo("RuntimeDispatcher writing package");

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
            Logger::logWarning("Local RuntimeDispatcher::processReading got invalid status, stopping reading: %s", status.error_message().c_str());
            return status;
        }
    }
    return Status::OK;
}

void RuntimeDispatcher::processPacket(DataPackage& pkt, Status& status) {
    status = Status::OK;
    Logger::logInfo("RunTimeDispatcher processPacket 1");
    // Enqueue the data package and we are all done.
    if (!pkt.has_control_val()) {
        // Check if the channel of the packet is valid. That means the
        // combination of channel, source, target and type has to exist in the
        // channel map.
        Logger::logInfo("RunTimeDispatcher processPacket 2");
        ChannelInfo chanInfo;
        auto chanEntry = moduleTable.isValidChannel(pkt);
        Logger::logInfo("RunTimeDispatcher processPacket 3");
        if (!chanEntry) {
            // TODO: Figure out how to handle errors without leaving
            // the read loop.
            Logger::logInfo("Received invalid data packet:");
            Logger::println(messageToString(pkt));
            return;
        }    
        Logger::logInfo("RunTimeDispatcher processPacket 4");


        moduleTable.addOutputPackets(pkt, chanEntry, incomingQueue);
        Logger::logInfo("RunTimeDispatcher processPacket 5");

        // Make a copy of the package and augment it with the
        // the fields not set by the client dispatcher.
        // auto cpPkt = make_shared<DataPackage>(pkt);
        // moduleTable.augmentFieldValues(*cpPkt, chanInfo);
        // incomingQueue.push_back(cpPkt);
        return;
    }
    Logger::logInfo("RunTimeDispatcher processPacket 6");

    // Simply forward control package. Will be processed by middleware.
    incomingQueue.push_back(std::make_shared<claidservice::DataPackage>(pkt));

    // // Process the control values
    // auto ctrlType = pkt.control_val().ctrl_type();
    // switch(ctrlType) {
    //     default: {
    //         status = Status(grpc::INVALID_ARGUMENT, "Invalid ctrl type");
    //         break;
    //     }
    // }
}

ServiceImpl::ServiceImpl(ModuleTable& modTable)
    : moduleTable(modTable) {}

Status ServiceImpl::GetModuleList(ServerContext* context,
        const ModuleListRequest* req, ModuleListResponse* resp) {
    if (req->runtime() == Runtime::RUNTIME_UNSPECIFIED) {
        return Status(grpc::INVALID_ARGUMENT, "Invalid runtime value provided");
    }

    // {
    //     std::string jsonOutput = "";
    //     google::protobuf::util::JsonPrintOptions options;
    //     options.add_whitespace = true;
    //     options.always_print_primitive_fields = true;
    //     options.preserve_proto_field_names = true;
    //     absl::Status status = MessageToJsonString(*req, &jsonOutput, options);
    //     Logger::logInfo("Received ModuleListRequest: %s\n", jsonOutput.c_str());
    // }

    set<string> supportedModClasses;
    // Register the runtime
    for(auto& it : req->supported_module_classes()) {
        // Look up the module class
        auto rt = moduleTable.moduleClassRuntimeMap[it];

        // The module class was already registered for this runtime
        // This is a no-op.
        if (rt != Runtime::RUNTIME_UNSPECIFIED && rt != req->runtime()) {
            return Status(grpc::INVALID_ARGUMENT, "Attempted redefiniton of the runtime for a module class");
        }
        
        moduleTable.moduleClassRuntimeMap[it] = req->runtime();
        supportedModClasses.insert(it);
    }
    std::map<std::string, claidservice::ModuleAnnotation> moduleAnnotations(req->module_annotations().begin(), req->module_annotations().end());
    moduleTable.appendModuleAnnotations(moduleAnnotations);

    // Iterate over the needed modules and only include the module classes
    // that are supported by the caller.
    for(auto it : moduleTable.moduleToClassMap) {
        if (supportedModClasses.find(it.second) != supportedModClasses.end()) {
            auto desc = resp->add_descriptors();
            desc->set_module_id(it.first);
            desc->set_module_class(it.second);
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
        return Status(grpc::INVALID_ARGUMENT, "Runtime identifier missing in InitRuntime.");
    }

    for(auto modChanIt : request->modules()) {
        // find module --> moduleClass --> runtime
        auto& moduleId = modChanIt.module_id();
        Logger::logInfo("InitRuntime: %s", moduleId.c_str());

        map<string, string>::const_iterator modClassIt;
        if ((modClassIt = moduleTable.moduleToClassMap.find(moduleId))== moduleTable.moduleToClassMap.end()) {
            return Status(grpc::INVALID_ARGUMENT, absl::StrCat("Unknown module id \"", moduleId, "\" given in InitRuntime called by Runtime ", Runtime_Name(rt)));
        }

        auto classRt = moduleTable.moduleClassRuntimeMap[modClassIt->second];
        if (classRt != rt) {
            return Status(grpc::INVALID_ARGUMENT, absl::StrCat("Module \"", moduleId, "\" was registered at Runtime ", Runtime_Name(classRt),
            "but now was requested to be loaded by Runtime ", Runtime_Name(rt), ". Runtime of Module does not the Runtime it was originally registered at."));
        }
        moduleTable.moduleRuntimeMap[moduleId] = classRt;
        if (!moduleTable.runtimeQueueMap[classRt]) {
            moduleTable.runtimeQueueMap[classRt] = make_shared<SharedQueue<DataPackage>>();
        }
        Logger::logInfo("InitRuntime num channel pakets for module %s: %d", moduleId.c_str(), modChanIt.channel_packets().size());

        // Add the channels for this module
        Status status = moduleTable.setChannelTypes(moduleId, modChanIt.channel_packets());
        if (!status.ok()) {
            return status;
        }
    }

    return Status::OK;
}

static unique_ptr<DataPackage> makeErrorPkt(const string& msg, const bool cancel) {
    auto pkt = make_unique<DataPackage>();
    auto ctrlMsg = pkt->mutable_control_val();
    ctrlMsg->set_ctrl_type(CTRL_ERROR);
    auto errMsg = ctrlMsg->mutable_error_msg();
    errMsg->set_message(msg);
    errMsg->set_cancel(cancel);
    return pkt;
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
        claid::Logger::logInfo("Returning with error %s \n", status.error_message().c_str());
        stream->Write(*makeErrorPkt(status.error_message(), true));
        return status;
    }

    claid::Logger::logInfo("Runtime dispatcher created");

    // TODO: Add cleanup function to clean up when we leave the scope.
    if (rtDispatcher->alreadyRunning()) {
        claid::Logger::logInfo("Runtime dispatcher already running");
        return Status(grpc::ALREADY_EXISTS, "Runtime dispatcher already exists");
    }
    dumpActiveDispatchers();
    claid::Logger::logInfo("Sending registration package");

    // Confirm that we are getting ready to write.
    DataPackage outPkt(inPkt);
    claid::Logger::logInfo("Sent registration package");
    stream->Write(outPkt);


    claid::Logger::logInfo("Start writer thread");
    rtDispatcher->startWriterThread(stream);

    claid::Logger::logInfo("Start reading");
    status = rtDispatcher->processReading(stream);
    claid::Logger::logInfo("Reading is done, shutting down writer thread.");
    rtDispatcher->shutdownWriterThread();

    removeRuntimeDispatcher(inPkt.control_val().runtime());

    dumpActiveDispatchers();
    return status;
}

void ServiceImpl::shutdown() {
    lock_guard<mutex> lock(adMutex);
    for(auto& it : activeDispatchers) {
        it.second->shutdownWriterThread();
    }
}

RuntimeDispatcher* ServiceImpl::addRuntimeDispatcher(DataPackage& pkt, Status& status) {
    status = Status::OK;
    auto runTime = pkt.control_val().runtime();

    // Make sure we got a control package with a CTRL_RUNTIME_PING message.
    if (!validCtrlRuntimePingPkt(pkt)) {
        status = Status(grpc::INVALID_ARGUMENT, absl::StrCat("Invalid control type or unspecified runtime %s %s", Runtime_Name(pkt.control_val().runtime()).c_str(), CtrlType_Name(pkt.control_val().ctrl_type())).c_str());
        return nullptr;
    }

    if(runTime == RUNTIME_UNSPECIFIED)
    {
        status = Status(grpc::INVALID_ARGUMENT, "Invalid runtime type RUNTIME_UNSPECIFIED.");
        return nullptr;
    }

    // check if the RuntimeDispatcher exits
    lock_guard<mutex> lock(adMutex);
    auto it = activeDispatchers.find(runTime);
    if (it != activeDispatchers.end()) {
        return it->second.get();
    }

    // Allocate a runtime Disptacher
    // TODO: avoid implict addition to map through [] operator.


    moduleTable.addRuntimeIfNotExists(runTime);

    shared_ptr<SharedQueue<DataPackage>> rtq = moduleTable.getOutputQueueOfRuntime(runTime);

    if(rtq == nullptr)
    {
        status = Status(grpc::NOT_FOUND, absl::StrCat("Unable to find Runtime \"", Runtime_Name(runTime), "\" in ModuleTable."));
        return nullptr;
    }

    Logger::logInfo("Rtq ptr: %lu", rtq.get());
    auto ret = new RuntimeDispatcher(moduleTable.inputQueue(), *rtq.get(), moduleTable);
    activeDispatchers[runTime] = unique_ptr<RuntimeDispatcher>(ret);
    return ret;
}

void ServiceImpl::removeRuntimeDispatcher(Runtime rt) {
    Logger::logInfo("Removing runtime dispatcher");
    lock_guard<mutex> lock(adMutex);
    activeDispatchers.erase(rt);
    Logger::logInfo("Removed runtime dispatcher");

}

void ServiceImpl::dumpActiveDispatchers() {
    lock_guard<mutex> lock(adMutex);
    claid::Logger::logInfo("Dispatchers: ");
    for(auto& it : activeDispatchers) {
        claid::Logger::logInfo("     %s", Runtime_Name(it.first).c_str());
    }
}

DispatcherServer::DispatcherServer(const string& addr, ModuleTable& modTable)
    : addr(addr), serviceImpl(modTable) {}

DispatcherServer::~DispatcherServer() {
    shutdown();
}

bool DispatcherServer::start() {
    buildAndStartServer();
    if (server) {
        claid::Logger::logInfo("Server listening on %s", addr.c_str());
    }
    return bool(server);
}

void DispatcherServer::shutdown() {
    if (!server) {
        return;
    }
    Logger::logInfo("DispatcherServer: Shutting down.");

    serviceImpl.shutdown();
    auto helperThread = make_unique<thread>([this]() {
        server->Wait();
    });

    Logger::logInfo("DispatcherServer: Shutting down server.");
    server->Shutdown();
    Logger::logInfo("DispatcherServer: DispatcherServer joining helper thread;");
    helperThread->join();
    Logger::logInfo("DispatcherServer: Releasing resources.");
    server = nullptr;
    Logger::logInfo("DispatcherServer: DispatcherServer shutdown successfully.");
}

void DispatcherServer::buildAndStartServer() {
    ServerBuilder builder;
    builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
    builder.RegisterService(&serviceImpl);
    builder.SetMaxReceiveMessageSize(1024 * 1024 * 1024);  // 1 GB
    // Set the maximum send message size (in bytes) for the server
    builder.SetMaxSendMessageSize(1024 * 1024 * 1024);  // 1 GB
    server = builder.BuildAndStart();
    }

DispatcherClient::DispatcherClient(const string& socketPath,
        SharedQueue<DataPackage>& inQueue,
        SharedQueue<DataPackage>& outQueue,
        const set<string>& supportedModClasses,
        const std::map<std::string, ModuleAnnotation>& moduleAnnotations) :
        incomingQueue(inQueue), outgoingQueue(outQueue),
        moduleClasses(supportedModClasses),
        moduleAnnotations(moduleAnnotations) {

    // Set up the gRCP channel
    grpc::ChannelArguments args;

    args.SetInt(GRPC_ARG_MAX_RECEIVE_MESSAGE_LENGTH, 1024 * 1024 * 1024);  // 1 GB
    args.SetInt(GRPC_ARG_MAX_SEND_MESSAGE_LENGTH, 1024 * 1024 * 1024);  // 1 GB

    grpcChannel = grpc::CreateCustomChannel(socketPath, grpc::InsecureChannelCredentials(), args);
    stub = ClaidService::NewStub(grpcChannel);

}


void DispatcherClient::shutdown() {
    if(!this->running)
    {
        return;
    }
    // Closing the outgoing queue will end the writer thread.
    Logger::logInfo("Dispatcher client shutdown 1");
    this->running = false;
    outgoingQueue.interruptOnce();
    if (writeThread) {
        writeThread->join();
        writeThread = nullptr;
    }
    Logger::logInfo("Dispatcher client shutdown 2");

    if (readThread) {
        readThread->join();
        readThread = nullptr;
    }
    Logger::logInfo("Dispatcher client shutdown 3");
}

unique_ptr<ModuleListResponse> DispatcherClient::getModuleList() {
    ClientContext context;
    ModuleListRequest req;
    req.set_runtime(Runtime::RUNTIME_CPP);
    for(auto it : moduleClasses) 
    {
        req.add_supported_module_classes(it);

        for(auto& moduleAnnotation : this->moduleAnnotations)
        {
            (*req.mutable_module_annotations())[moduleAnnotation.first] = moduleAnnotation.second;
        }
        
    }

    unique_ptr<ModuleListResponse> resp = make_unique<ModuleListResponse>();
    Status status = stub->GetModuleList(&context, req, resp.get());

    return (status.ok()) ? (std::move(resp)) : (unique_ptr<ModuleListResponse>(nullptr));
}

void makeControlRuntimePing(ControlPackage& pkt) {
    pkt.set_ctrl_type(CtrlType::CTRL_RUNTIME_PING);
    pkt.set_runtime(Runtime::RUNTIME_CPP);
}

bool DispatcherClient::startRuntime(const InitRuntimeRequest& req) {
    // Initialize the runtime
    {
        ClientContext context;
        Empty empty;
        Status status = stub->InitRuntime(&context,req, &empty);
        if (!status.ok()) {
            claid::Logger::logInfo("Could not init request: %s", status.error_message().c_str());
            return false;
        }
    }

    // Start the bidirectional request.
    streamContext = make_shared<ClientContext>();
    stream = stub->SendReceivePackages(streamContext.get());

    // Send the ping message to the server.
    claidservice::DataPackage pingReq;
    makeControlRuntimePing(*pingReq.mutable_control_val());
    if (!stream->Write(pingReq)) {
        claid::Logger::logInfo("Failed sending ping package to server.");
        return false;
    }

    // Wait for the valid response ping
    DataPackage pingResp;
    if (!stream->Read(&pingResp)) {
        claid::Logger::logInfo("Did not receive a ping package from server !");
        return false;
    }

    // TODO: @Stephan, shouldn't this here be pingResp ?
    // @Patrick:
    // Check whether the package read was a control package with the right type.
    if (pingReq.control_val().ctrl_type() != CtrlType::CTRL_RUNTIME_PING) {
        return false;
    }
    this->running = true;

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
    while(this->running) {

        auto pkt = outgoingQueue.interruptable_pop_front();
        if (!pkt) {
            if(outgoingQueue.is_closed())
            {
                break;
            }
        } else {
            if (!stream->Write(*pkt)) {
                claid::Logger::logInfo("Client: Error writing packet");
                break;
            }        
        }
    }
    Logger::logInfo("DispatcherClient writes done");
    stream->WritesDone();
    auto status = stream->Finish();
    if (!status.ok()) {
        claid::Logger::logInfo("Got error finishing the writing: %s", status.error_message().c_str());
    }
}
