#ifndef MIDDLEWARE_HH__
#define MIDDLEWARE_HH__

#include <string>
#include <memory>
#include <thread>

#include "absl/status/status.h"
#include "dispatch/core/module_table.hh"
#include "dispatch/core/local_dispatching.hh"
#include "dispatch/core/Configuration/Configuration.hh"

#include "dispatch/core/RemoteDispatching/RemoteDispatcherClient.hh"
#include "dispatch/core/RemoteDispatching/RemoteDispatcherServer.hh"
#include "dispatch/core/RemoteDispatching/HostUserTable.hh"
#include "dispatch/core/RemoteDispatching/ClientTable.hh"
#include "dispatch/core/Router/MasterRouter.hh"
#include "dispatch/core/Router/LocalRouter.hh"
#include "dispatch/core/Router/ClientRouter.hh"
#include "dispatch/core/Router/ServerRouter.hh"
#include "dispatch/core/Router/RoutingQueueMerger.hh"
#include "dispatch/core/Router/RoutingQueueMergerGeneric.hh"

#include "dispatch/core/Logger/LogSinkConfiguration.hh"

using claidservice::ConfigUploadPayload;

namespace claid
{
    // Top container of the CLAID middleware.
    // TODO: This should probably merged into CLAID.*
    class MiddleWare {
        public:
            MiddleWare(
                const std::string& socketPath,
                const std::string& configurationPath,
                const std::string& hostId,
                const std::string& userId,
                const std::string& deviceId
            );

            absl::Status start();
            absl::Status shutdown();

            const std::string& getSocketPath() const;
            const std::string& getHostId() const;
            const std::string& getUserId() const;
            const std::string& getDeviceId() const;

            int getLogSinkSeverityLevel() const;

            bool isConnectedToRemoteServer() const;
            absl::Status getRemoteClientStatus() const;

            absl::Status loadNewConfig(const std::string& configPath);
            absl::Status loadNewConfig(const Configuration& configuration);

            void setPayloadDataPath(const std::string& path);
            const std::string& getPayloadDataPath() const;
            void enableDesignerMode();
            void disableDesignerMode();

            virtual ~MiddleWare();


        private:

            bool running = false;

            bool waitingForAllRuntimesToUnloadModules = false;
            size_t numberOfRuntimesThatUnloadedModules = 0;
            bool modulesUnloaded = false;

            bool waitingForAllRuntimesToRestart = false;
            size_t numberOfRuntimesThatRestarted = 0;

            // Path to specify where additional payload data is stored when a control message
            // of type UPLOAD_CONFIG_AND_DATA is received. Requires designerMode to be activated in the configuration file.
            std::string payloadDataPath = "";
            bool designerModeActive = false;

            // TODO: Incorporate all variables into the code if necessary.
            std::string socketPath;
            std::string configurationPath;
            std::string hostId;
            std::string userId;
            std::string deviceId;

            Configuration currentConfiguration;
            LogSinkConfiguration logSinkConfiguration;

            ModuleTable moduleTable;
            HostUserTable hostUserTable;
            ClientTable clientTable;
                
            // Server for local runtimes (C++, Dart, Java, Python, ...) to connect to.
            std::unique_ptr<DispatcherServer> localDispatcher;

            // Server for external instances of CLAID to connect to.
            std::unique_ptr<RemoteDispatcherServer> remoteDispatcherServer;

            // Client for the current instance of CLAID to connect to another instance of CLAID.
            std::unique_ptr<RemoteDispatcherClient> remoteDispatcherClient;

            RoutingTree routingTree;

            // 
            std::unique_ptr<MasterRouter> masterRouter;
            
            // Merges the queus from the localDispatcher, remoteDispatcherServer and remoteDispatcherClient into a single queue.
            // This queue is processed by the masterRouter, which decides whether to route the package locally (to a runtime),
            // to a connected client (downwards in the routing tree) or to a connected server (upwards in the routing tree).
            std::unique_ptr<RoutingQueueMerger> routingQueueMerger;
            SharedQueue<DataPackage> masterInputQueue;

            std::shared_ptr<SharedQueue<LogMessage>> logMessagesQueue;

            std::unique_ptr<std::thread> controlPackageHandlerThread = nullptr; 

            // Thread needed to load a new config if a message CTRL_UPLOAD_CONFIG_AND_DATA is received.
            // Loading a new config blocks the thread that calls loadNewConfig until the new config is loaded successfully.
            // Normally, this is fine and expected, since the thread calling loadNewConfig is not the same thread that processes control messages.
            // However if another host wants to upload a new config remotely, he would send a control message CTRL_UPLOAD_CONFIG_AND_DATA.
            // This message is processed in the thread processing control packages, which would then need to call loadNewConfig.
            // However, loadNewConfig is blocking and waiting until all runtimes have sent the control messages CTRL_UNLOAD_MODULES_DONE and CTRL_RESTART_RUNTIME_DONE
            // However, these messages would never be processed, since the thread processing control packages is still blocked on loadNewConfig -> deadlock.
            // Hence, in this case we use a separate helper thread.
            std::unique_ptr<std::thread> configUploadThread;
            bool uploadingConfigFinished = false;
            DataPackage configUploadPackage;

            absl::Status getHostModuleAndChannelDescriptions(const std::string& hostId, const Configuration& config,
                HostDescriptionMap& hostDescriptions, ModuleDescriptionMap& allModuleDescriptions,
                ModuleDescriptionMap& hostModuleDescriptions, ChannelDescriptionMap& channelDescriptions);

            absl::Status populateModuleTable(
                const ModuleDescriptionMap& allModuleDescriptions,
                const ModuleDescriptionMap& moduleDescriptions,
                const ChannelDescriptionMap& channelDescriptions,
                claid::ModuleTable& moduleTable);

            absl::Status startRemoteDispatcherServer(const std::string& currentHost, const HostDescriptionMap& hostDescriptions);
            
            absl::Status startRemoteDispatcherClient(const std::string& currentHost, const std::string& currentUser, 
                const std::string& currentDeviceId, const HostDescriptionMap& hostDescriptions);
            
            absl::Status startRouter(const std::string& currentHost, const HostDescriptionMap& hostDescriptions, const ModuleDescriptionMap& moduleDescriptions);

            void setupLogSink();

            absl::Status unloadAllModulesInAllLocalRuntimes();
            absl::Status loadNewConfigIntoModuleTableAndRouter(const Configuration& config);
            absl::Status restartRuntimesWithNewConfig();

            void readControlPackages();
            void handleControlPackage(std::shared_ptr<DataPackage> controlPackage);
            void forwardControlPackageToAllRuntimes(std::shared_ptr<DataPackage> package);
            void forwardControlPackageToTargetRuntime(std::shared_ptr<DataPackage> package);

            void readLogMessages();
            std::shared_ptr<SharedQueue<LogMessage>> getLogMessagesQueue();


            void handleUploadConfigAndPayloadMessage();
            bool storePayload(const ConfigUploadPayload& payload);
            void notifyAllRuntimesAboutNewPayload();
    };
}

#endif    //  MIDDLEWARE_HH__

