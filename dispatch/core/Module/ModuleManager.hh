#pragma once

#include "dispatch/core/shared_queue.hh"
#include "dispatch/proto/claidservice.grpc.pb.h"
#include "dispatch/core/local_dispatching.hh"
#include "dispatch/core/Module/Module.hh"
#include "dispatch/core/Module/ModuleFactory/ModuleFactory.hh"
#include "dispatch/core/proto_util.hh"

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

            ChannelSubscriberPublisher subscriberPublisher;

            // ModuleId, Module
            std::map<std::string, std::unique_ptr<Module>> runningModules;

            bool running = false;

            std::thread fromModuleDispatcherReaderThread;
            
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

            void shutdownModules();

        public:
            ModuleManager(DispatcherClient& dispatcher,
                SharedQueue<DataPackage>& fromModuleDispatcherQueue,
                SharedQueue<DataPackage>& toModuleDispatcherQueue);
            

            absl::Status start();
            void stop();
          
           

    };
}