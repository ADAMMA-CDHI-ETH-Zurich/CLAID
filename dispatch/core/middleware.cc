#include "dispatch/core/middleware.hh"
#include "dispatch/core/Configuration/Configuration.hh"
#include "dispatch/core/CLAID.hh"

using namespace claid;
using namespace std;

MiddleWare::MiddleWare(const string& socketPath, const string& configurationPath,
    const string& currentHost, const string& currentUser, const string& currentDeviceId)
        : socketPath(socketPath), configurationPath(configurationPath),
          currentHost(currentHost), currentUser(currentUser), currentDeviceId(currentDeviceId)
          {}

MiddleWare::~MiddleWare() {
    auto status = shutdown();
    if (status.ok()) {
        // TODO: replace with proper logging
        cout << "Error running shutdown in MiddleWare destructore." << endl;
    }
}

absl::Status MiddleWare::start() {
    // Fill the module table fromt he config file
    Configuration config;
    auto status = config.parseFromJSONFile(configurationPath);
    if (!status.ok()) {
        return status;
    }

    ModuleDescriptionMap moduleDescriptions;
    ChannelDescriptionMap channelDescriptions;
    status = config.getModuleDescriptions(moduleDescriptions);
    if (!status.ok()) {
        return status;
    }

    status = config.getChannelDescriptions(channelDescriptions);
    if (!status.ok()) {
        return status;
    }

    status = populateModuleTable(moduleDescriptions, channelDescriptions, moduleTable);
    if (!status.ok()) {
        return status;
    }

    localDispatcher = make_unique<DispatcherServer>(socketPath, moduleTable);
    if (!localDispatcher->start()) {
        return absl::InternalError("Error starting local dispatcher !");
    }

    // TODO: REMOVE
    // This is only temporary for testing against Dart --> this should
    // be replaced by the router.
    routerThread = make_unique<thread>([this]() {
        while(true) {
            auto pkt = moduleTable.inputQueue().pop_front();
            if (!pkt || moduleTable.inputQueue().is_closed()) {
                return;
            }
            auto outQ = moduleTable.lookupOutputQueue(pkt->target_host_module());
            if (outQ) {
                outQ->push_back(pkt);
            }
        }
    });

    // HostDescriptionMap hostDescriptions;
    // status = config.getHostDescriptions(hostDescriptions);
    // if (!status.ok()) {
    //     return status;
    // }
    
    // status = startRemoteDispatcherServer(currentHost, hostDescriptions);
    // if(!status.ok())
    // {
    //     return status;
    // }

    // status = startRemoteDispatcherClient(currentHost, currentUser, currentDeviceId, hostDescriptions);
    // if(!status.ok())
    // {
    //     return status;
    // }

    // status = this->startRouter(currentHost, hostDescriptions);
    // if (!status.ok()) {
    //     return status;
    // }

    return absl::OkStatus();
}

absl::Status MiddleWare::startRemoteDispatcherServer(const std::string& currentHost, const HostDescriptionMap& hostDescriptions)
{
    if(this->remoteDispatcherServer != nullptr)
    {
        return absl::AlreadyExistsError("Failed to start server; RemoteDispatcherServer already exists.");
    }

    // Lookup address of currentHost.
    auto it = hostDescriptions.find(currentHost);
    if(it == hostDescriptions.end())
    {
        return absl::NotFoundError(absl::StrCat(
            "Failed to lookup server address for host for host \"", currentHost, "\".",
            "The host was not found in the configuration file."
        ));
    }
    
    const HostDescription& hostDescription = it->second;
    if(!hostDescription.isServer)
    {
        return absl::InvalidArgumentError(absl::StrCat(
            "Cannot start RemoteDispatcherServer on host \"", currentHost, "\".\n",
            "The host was not configured as server (isServer = false)."
        ));
    }

    const std::string address = hostDescription.hostServerAddress;

    this->remoteDispatcherServer = make_unique<RemoteDispatcherServer>(address, this->hostUserTable);
    
    return this->remoteDispatcherServer->start();
}

absl::Status MiddleWare::startRemoteDispatcherClient(const std::string& currentHost, const std::string& currentUser, 
                const std::string& currentDeviceId, const HostDescriptionMap& hostDescriptions)
{
    if(this->remoteDispatcherServer != nullptr)
    {
        return absl::AlreadyExistsError("Failed to start client for connection to external server; RemoteDispatcherClient already exists.");
    }
    
    //     claid::RemoteDispatcherClient client(address, host, userToken, deviceID, inQueue, outQueue);
    // absl::Status status = client.registerAtServerAndStartStreaming();

    return absl::OkStatus();
}

absl::Status MiddleWare::startRouter(const std::string& currentHost, const HostDescriptionMap& hostDescriptions)
{
    if(this->routingQueueMerger != nullptr)
    {
        return absl::AlreadyExistsError("Failed to start router: RoutingQueueMerger already exists.");
    }

    auto genericMerger = new RoutingQueueMergerGeneric(this->masterInputQueue, this->moduleTable.inputQueue(), this->hostUserTable.inputQueue());
    std::unique_ptr<typename std::remove_pointer<decltype(genericMerger)>::type> uniqueMerger(genericMerger);
    this->routingQueueMerger = std::move(uniqueMerger);
    
    absl::Status status = this->routingQueueMerger->start();
    if(!status.ok())
    {
        return status;
    }

    std::shared_ptr<LocalRouter> localRouter = std::make_shared<LocalRouter>(this->moduleTable);
    std::shared_ptr<ServerRouter> serverRouter = std::make_shared<ServerRouter>(this->hostUserTable);
    std::shared_ptr<ClientRouter> clientRouter = std::make_shared<ClientRouter>();

    MasterRouter router(this->masterInputQueue, localRouter, clientRouter, serverRouter);
    status = router.buildRoutingTable(currentHost, hostDescriptions);
    if(!status.ok())
    {
        return status;
    }

    status = router.start();
    if(!status.ok())
    {
        return status;
    }

    return absl::OkStatus();
}

absl::Status MiddleWare::shutdown() 
{
    // TODO: remove once the router is added.

    if (localDispatcher) {
        if (routerThread) {
            moduleTable.inputQueue().close();
            routerThread->join();
            routerThread.reset();
        }
        localDispatcher.reset();
    }
    return absl::OkStatus();
}

