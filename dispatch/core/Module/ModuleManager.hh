#pragma once

#include "dispatch/core/shared_queue.hh"
#include "dispatch/proto/claidservice.grpc.pb.h"
#include "dispatch/core/local_dispatching.hh"
#include "dispatch/core/Module/Module.hh"
#include "dispatch/core/Module/ModuleFactory/ModuleFactory.hh"

using claidservice::DataPackage;
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

            std::thread fromModulesReaderThread;
            std::thread fromModuleDispatcherReaderThread;

        public:
            ModuleManager(DispatcherClient& dispatcher,
                SharedQueue<DataPackage>& fromModuleDispatcherQueue,
                SharedQueue<DataPackage>& toModuleDispatcherQueue) : dispatcher(dispatcher), 
                                                                    fromModuleDispatcherQueue(fromModuleDispatcherQueue), 
                                                                    toModuleDispatcherQueue(toModuleDispatcherQueue),
                                                                    subscriberPublisher(toModuleDispatcherQueue)
            {

            }

        //     absl::Status instantiateModule(const std::string& moduleId, const std::string& moduleClass)
        //     {
        //         ModuleFactory& moduleFactory = *ModuleFactory::getInstance();

        //         if(!moduleFactory.isModuleClassRegistered(moduleClass))
        //         {
        //             return absl::NotFoundError(absl::StrCat("ModuleManager: Failed to instantiate Module of class \"", moduleClass, "\" (id: ", moduleId, "\")\n",
        //             "A Module with this class is not registered to the ModuleFactory."));
                
        //             // The Middleware always returns all Modules named in the config file.
        //             // If we cannot load it, we assume another registered runtime can.
        //             // Therefore, no error.
        //           //  return true;
        //         }

        //         Logger::logInfo("Loaded Module id \"%s\" (class: \"%s\").", moduleId.c_str(), moduleClass.c_str());
        //         std::unique_ptr<Module> moduleInstance(moduleFactory.getInstanceUntyped(moduleClass));

        //         moduleInstance->setId(moduleId);
        //         this->runningModules.insert(make_pair(moduleId, std::move(moduleInstance)));
        //         return absl::OkStatus();
        //     }

        //     absl::Status instantiateModules(const ModuleListResponse& moduleList)
        //     {
        //         for(ModuleListResponse_ModuleDescriptor descriptor : moduleList.descriptors())
        //         {
        //             const std::string& moduleId = descriptor.module_id();
        //             const std::string& moduleClass = descriptor.module_class();

        //             absl::Status status = this->instantiateModule(moduleId, moduleClass);
        //             if(!status.ok())
        //             {
        //                 return status;
        //             }
        //         }
        //         return absl::OkStatus();
        //     }

        //     absl::Status initializeModules(ModuleListResponse moduleList, ChannelSubscriberPublisher subscriberPublisher)
        //     {
        //         for(ModuleListResponse_ModuleDescriptor descriptor : moduleList.descriptors())
        //         {
        //             const std::string& moduleId = descriptor.module_id();
        //             const std::string& moduleClass = descriptor.module_class();

        //             const std::string& key = moduleId;
        //             if(this->runningModules.find(key) == this->runningModules.end())
        //             {
        //                 return absl::NotFoundError(absl::StrCat(
        //                     "Failed to initialize Module \"" + moduleId + "\" (class: \"" + moduleClass + "\".\n" +
        //                     "The Module was not loaded."
        //                 ));
                        
        //                 // The middleware always sends all Modules parsed from the configuration file.
        //                 // If this Module is not running in this runtime, that means we could not instantiate it before.
        //                 // Hence, we assume this Module is supported by another runtime.
        //                 // continue;
        //             }

        //             std::unique_ptr<Module>& module = this->runningModules[key];

                    
        //             // This will call the initialize functions of each Module.
        //             // In the initialize function (and ONLY there), Modules can publish or subscribe Channels.
        //             // Hence, once all Modules have been initialized, we know the available Channels.
        //             Logger::logInfo("Calling module.start() for Module \"%s\".", moduleId);
        //             std::map<std::string, std::string> properties;

        //             properties.insert(descriptor.properties().begin(), descriptor.properties().end());

        //             module->start(&subscriberPublisher, properties);
        //             Logger::logInfo("Module \"%s\" has started.",  moduleId.c_str());

        //         }
        //         return absl::OkStatus();
        //     }

        //     // A Channel is defined by a template package defining source | target | payload.
        //     // For each Module, we have a list of DataPackages defining all Channels published or subscribed by the Module.
        //     // This functions builds a Map<Module, List<DataPackage>>, representing the (data packages for each) channel of the Module.
        //     void getTemplatePackagesOfModules(std::map<std::string, std::vector<DataPackage>>& moduleChannels)
        //     {
        //         moduleChannels.clear();
        //         for(const auto& entry : this->runningModules)
        //         {
        //             const std::string& moduleId = entry.first;

        //             std::vector<DataPackage> templatePackagesForModule = this->subscriberPublisher.getChannelTemplatePackagesForModule(moduleId);
        //             moduleChannels.insert(make_pair(moduleId, templatePackagesForModule));
        //         }
        //     }

        //     InitRuntimeRequest makeInitRuntimeRequest()
        //     {
        //         InitRuntimeRequest initRuntimeRequest;
        //         std::map<std::string, std::vector<DataPackage>> moduleTemplatePackages;
        //         getTemplatePackagesOfModules(moduleTemplatePackages);
        //         for(const auto& entry : moduleTemplatePackages)
        //         {
        //             InitRuntimeRequest_ModuleChannels moduleChannels;

        //             moduleChannels.set_module_id(entry.first);
        //             moduleChannels.clear_channel_packets();

        //             for(const DataPackage& package : entry.second)
        //             {
        //                 *moduleChannels.add_channel_packets() = package;
        //             }

        //             *initRuntimeRequest.add_modules() = moduleChannels;
        //         }   
        //         initRuntimeRequest.set_runtime(Runtime::RUNTIME_CPP);
        //         return initRuntimeRequest;
        //     }

        //     absl::Status start()
        //     {
        //         if(this->running)
        //         {
        //             return absl::AlreadyExistsError("Cannot start C++ Module manager. Start has been called twice.");
        //         }


        //     //    this->dispatcher = std::make_unique<DispatcherClient>(socketPath, fromModuleDispatcherQueue, toModuleDispatcherQueue, );
        //         std::unique_ptr<ModuleListResponse> moduleList = this->dispatcher.getModuleList();
                
        //         absl::Status status;

        //         status = instantiateModules(*moduleList);
        //         if(!status.ok())
        //         {
        //             return status;
        //         }


        //         status = initializeModules(*moduleList, subscriberPublisher);
        //         if(!status.ok())
        //         {
        //             return status;
        //         }

          
        //         InitRuntimeRequest initRuntimeRequest = makeInitRuntimeRequest();

        //         if(!this->dispatcher.startRuntime(initRuntimeRequest))
        //         {
        //             return absl::InvalidArgumentError("Failed to start C++ Runtime.");
        //         }

        //         this->readFromModulesThread.start();

        //         this->running = true;
        //         // Map<String: moduleId, List<DataPackage>: list of channels
        // /*        Map<String, List<DataPackage>> modules;

        //         if(!startRuntime())
        //         {
        //             return false;
        //         } */

        //         // init runtime
        //         // sendreceivepackafges
        //         // process queues

        //     }

        //     void readFromModules()
        //     {
        //         while(this->running)
        //         {

        //         }
        //     }

        //     void readFromModulesDispatcher()
        //     {
        //         while(this->running)
        //         {

        //         }
        //     }
         
    };
}