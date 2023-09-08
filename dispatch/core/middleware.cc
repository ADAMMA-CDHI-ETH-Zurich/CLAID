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

    return absl::OkStatus();
}

absl::Status MiddleWare::shutdown() {
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

