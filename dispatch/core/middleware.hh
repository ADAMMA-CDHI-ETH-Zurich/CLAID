/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Stephan Altmüller
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*         http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
***************************************************************************/

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

#include "dispatch/core/EventTracker/EventTracker.hh"
#include "dispatch/core/Logger/LogSinkConfiguration.hh"

#include "dispatch/core/RemoteFunction/RemoteFunctionHandler.hh"
#include "dispatch/core/RemoteFunction/RemoteFunctionRunnableHandler.hh"

#include "dispatch/core/DeviceScheduler/GlobalDeviceScheduler.hh"

#ifdef __ANDROID__
    #include "dispatch/core/DeviceScheduler/GlobalDeviceSchedulerAndroid.hh"
#endif

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

            bool isConnectedToRemoteServer();
            absl::Status getRemoteClientStatus() const;

            absl::Status loadNewConfig(const std::string& configPath);
            absl::Status loadNewConfig(const Configuration& configuration);

            void setPayloadDataPath(const std::string& path);
            const std::string& getPayloadDataPath() const;

            void setCommonDataPath(const std::string& path);
            const std::string& getCommonDataPath() const;

            void enableDesignerMode();
            void disableDesignerMode();

            std::shared_ptr<EventTracker> getEventTracker();

            virtual ~MiddleWare();

            // RPCs
            std::map<std::string, std::string> getAllRunningModulesOfAllRuntimes();
            bool addLooseDirectSubscription(claidservice::LooseDirectChannelSubscription subscription);
            bool addLooseDirectSubscriptionIfNotExists(claidservice::LooseDirectChannelSubscription subscription);
            void removeLooseDirectSubscription(claidservice::LooseDirectChannelSubscription subscription); 

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

            // Public path for CLAID to store data, like event logs.
            std::string commonDataPath = "";

            // TODO: Incorporate all variables into the code if necessary.
            std::string socketPath;
            std::string configurationPath;
            std::string hostId;
            std::string userId;
            std::string deviceId;

            Configuration currentConfiguration;

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

            std::mutex logSinkConfigurationMutex;
            LogSinkConfiguration logSinkConfiguration;
            std::shared_ptr<SharedQueue<LogMessage>> logMessagesQueue;
            std::unique_ptr<std::thread> logMessageHandlerThread = nullptr;
            std::set<Runtime> runtimesThatSubscribedToLogSinkStream;

            std::unique_ptr<std::thread> controlPackageHandlerThread = nullptr; 

            RemoteFunctionRunnableHandler remoteFunctionRunnableHandler;
            RemoteFunctionHandler remoteFunctionHandler; 
            std::shared_ptr<GlobalDeviceScheduler> globalDeviceScheduler = nullptr;

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

            std::shared_ptr<EventTracker> eventTracker;

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
            void assertAllModulesLoaded();

            absl::Status unloadAllModulesInAllLocalRuntimes();
            absl::Status loadNewConfigIntoModuleTableAndRouter(const Configuration& config);
            absl::Status restartRuntimesWithNewConfig();

            void readControlPackages();
            void handleControlPackage(std::shared_ptr<DataPackage> controlPackage);
            void forwardControlPackageToAllRuntimes(std::shared_ptr<DataPackage> package);
            void forwardControlPackageToTargetRuntime(std::shared_ptr<DataPackage> package);
            void forwardControlPackageToSpecificRuntime(std::shared_ptr<DataPackage> package, Runtime runtime);

            void handleRPCModuleNotFoundError(std::shared_ptr<DataPackage> rpcRequestPackage);

            void readLocalLogMessages();
            void handleLocalLogMessage(std::shared_ptr<LogMessage> logMessage);
            void forwardLogSinkLogStreamMessageToRuntimesThatSubscribed(std::shared_ptr<DataPackage> package);

            void handleUploadConfigAndPayloadMessage();
            bool storePayload(const ConfigUploadPayload& payload);
            void notifyAllRuntimesAboutNewPayload(const ConfigUploadPayload& payload);

            void createPlatformSpecificGlobalDeviceScheduler();

    };
}

#endif    //  MIDDLEWARE_HH__

