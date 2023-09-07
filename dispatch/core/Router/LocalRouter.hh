#pragma once

#include "dispatch/core/Router/Router.hh"
#include "dispatch/core/module_table.hh"
#include "dispatch/core/Logger/Logger.hh"
namespace claid
{
    // The LocalRouter is invoked from the MasterRouter, if a package is targeted
    // for a Module on the current host (this host). 
    // The LocalRouter then routes the package to the correct RuntimeDispatcher via the corresponding queues.

    class LocalRouter final : public Router
    {
        private:
            void routePackage(std::shared_ptr<DataPackage> dataPackage) override final
            {
                const std::string& targetHostModule = dataPackage->target_host_module();

                std::string targetHost;
                std::string targetModule;

                absl::Status status = getTargetHostAndModule(*dataPackage.get(), targetHost, targetModule);

                if(!status.ok())
                {
                    Logger::printfln("LocalRouter: Failed to route package to local Module.\n"
                    "Unable to split target address \"%s\" into host and module.\n"
                    "Make sure that target address is in format host:module.", targetHostModule.c_str());
                    return;
                }


                claidservice::Runtime targetRuntime;
                if(!getRuntimeOfModule(targetModule, targetRuntime))
                {
                    Logger::printfln("LocalRouter: Failed to route package to local Module.\n"
                    "Unable to find any Runtime where a Module with id \"%s\" is running in.\n"
                    "It seems the Module is not active or does not exist.", targetModule.c_str());
                    return;
                }

                std::shared_ptr<SharedQueue<claidservice::DataPackage>> runTimeInputQueue;

                if(!getInputQueueOfRuntime(targetRuntime, runTimeInputQueue))
                {
                    const std::string& runtimeName = claidservice::Runtime_Name(targetRuntime);

                    Logger::printfln("LocalRouter: Failed to route package to local Module running in RunTime \"%s\".\n"
                    "Unable to get input queue of the Runtime. Possibly, the Runtime was not registered.", runtimeName.c_str(), targetModule.c_str());
                    return;
                }

                runTimeInputQueue->push_back(dataPackage);
            }

            bool getRuntimeOfModule(const std::string& moduleName, claidservice::Runtime& runtime)
            {
                auto it = this->moduleTable.moduleRuntimeMap.find(moduleName);

                if(it == this->moduleTable.moduleRuntimeMap.end())
                {
                    runtime = claidservice::Runtime::RUNTIME_UNSPECIFIED;
                    return false;
                }

                runtime = it->second;

                return true;
            }

            bool getInputQueueOfRuntime(
                claidservice::Runtime runtime, 
                std::shared_ptr<SharedQueue<claidservice::DataPackage>>& queue)
            {
                auto it = this->moduleTable.runtimeQueueMap.find(runtime);

                if(it == this->moduleTable.runtimeQueueMap.end())
                {
                    queue = nullptr;
                    return false;
                }
                
                queue = it->second;

                return true;
            }


            const ModuleTable& moduleTable;

        public:
            LocalRouter(
                    SharedQueue<claidservice::DataPackage>& incomingQueue, 
                    const ModuleTable& moduleTable) : Router(incomingQueue), moduleTable(moduleTable)
            {

            }
        
    };
}