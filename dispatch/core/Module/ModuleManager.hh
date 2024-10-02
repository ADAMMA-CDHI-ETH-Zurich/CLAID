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

#pragma once

#include "dispatch/core/shared_queue.hh"
#include "dispatch/proto/claidservice.grpc.pb.h"
#include "dispatch/core/local_dispatching.hh"
#include "dispatch/core/Module/Module.hh"
#include "dispatch/core/Module/ModuleFactory/ModuleFactory.hh"
#include "dispatch/core/proto_util.hh"
#include "dispatch/core/EventTracker/EventTracker.hh"
#include "dispatch/core/Module/Properties.hh"
#include "dispatch/core/RemoteFunction/RemoteFunctionHandler.hh"
#include "dispatch/core/RemoteFunction/RemoteFunctionRunnableHandler.hh"
#include "dispatch/core/module_table.hh"


using claidservice::DataPackage;
using claidservice::ControlPackage;
using claidservice::CtrlType;
using claidservice::ModuleListResponse;
using claidservice::ModuleListResponse_ModuleDescriptor;
using claidservice::InitRuntimeRequest;
using claidservice::InitRuntimeRequest_ModuleChannels;
using claidservice::Runtime;

namespace claid {


    class ModuleManager
    {
        private:
            DispatcherClient& dispatcher;

            SharedQueue<DataPackage>& fromModuleDispatcherQueue;
            SharedQueue<DataPackage>& toModuleDispatcherQueue;

            std::shared_ptr<EventTracker> eventTracker;

            ChannelSubscriberPublisher subscriberPublisher;

            RemoteFunctionHandler remoteFunctionHandler;
            RemoteFunctionRunnableHandler remoteFunctionRunnableHandler;

            // ModuleId, Module
            std::map<std::string, std::unique_ptr<Module>> runningModules;

            bool running = false;

            std::thread fromModuleDispatcherReaderThread;

            std::unique_ptr<std::thread> restartThread;
            DataPackage restartControlPackage;
            
            absl::Status instantiateModule(const std::string& moduleId, const std::string& moduleClass);
            absl::Status instantiateModules(const ModuleListResponse& moduleList);
            absl::Status initializeModules(const ModuleListResponse& moduleList, ChannelSubscriberPublisher& subscriberPublisher);

            // A Channel is defined by a template package defining source | target | payload.
            // For each Module, we have a list of DataPackages defining all Channels published or subscribed by the Module.
            // This functions builds a Map<Module, List<DataPackage>>, representing the (data packages for each) channel of the Module.
            void getTemplatePackagesOfModules(std::map<std::string, std::vector<DataPackage>>& moduleChannels);

            InitRuntimeRequest makeInitRuntimeRequest();

            void readFromModulesDispatcher();
            void onPackageReceivedFromModulesDispatcher(std::shared_ptr<DataPackage> dataPackage);
            void handlePackageWithControlVal(std::shared_ptr<DataPackage> package);

            void handleRemoteFunctionRequest(std::shared_ptr<DataPackage> remoteFunctionRequest);
            void handleRuntimeRemoteFunctionExecution(std::shared_ptr<DataPackage> request);
            void handleModuleRemoteFunctionExecution(std::shared_ptr<DataPackage> request);
            void handleRemoteFunctionResponse(std::shared_ptr<DataPackage> remoteFunctionResponse);

            void shutdownModules();

            void restart();


        public:
            ModuleManager(DispatcherClient& dispatcher,
                SharedQueue<DataPackage>& fromModuleDispatcherQueue,
                SharedQueue<DataPackage>& toModuleDispatcherQueue,
                std::shared_ptr<EventTracker> eventTracker);

            ~ModuleManager();
            

            absl::Status start();
            void stop();

            RemoteFunctionHandler* getRemoteFunctionHandler();
    };
}