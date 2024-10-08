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

#include "dispatch/core/CLAID.hh"
#include "dispatch/core/Router/RoutingQueueMergerGeneric.hh"
#include "dispatch/core/Router/MasterRouter.hh"
#include "dispatch/core/Configuration/Configuration.hh"
#include "dispatch/core/RemoteDispatching/HostUserTable.hh"
#include "dispatch/core/Logger/Logger.hh"


namespace claid {
    // bool CLAID::started = false;

    // std::unique_ptr<DispatcherClient> CLAID::moduleDispatcher;
    // std::unique_ptr<ModuleManager> CLAID::moduleManager;

    // SharedQueue<DataPackage> CLAID::fromModuleDispatcherQueue;
    // SharedQueue<DataPackage> CLAID::toModuleDispatcherQueue;

    // void* CLAID::handle = nullptr;
    
    

    bool CLAID::start(const std::string& socketPath, const std::string& configFilePath, 
        const std::string& hostId, const std::string& userId, const std::string& deviceId, std::string commonDataPath)
    {
        Logger::logInfo("CLAID start");

        if(started)
        {
            startStatus = absl::UnavailableError("CLAID middleware start was called twice!");
            Logger::logError("%s", startStatus.ToString().c_str());
            return false;
        }
        
        handle = start_core_with_event_tracker(socketPath.c_str(), configFilePath.c_str(), 
            hostId.c_str(), userId.c_str(), deviceId.c_str(), commonDataPath.c_str());
        
        if(handle == 0)
        {
            startStatus = absl::InvalidArgumentError("Failed to start CLAID middleware core (start_core), returned handle is 0.");
            Logger::logError("%s", startStatus.ToString().c_str());
            return false;
        }

        if(!attachCppRuntime(handle))
        {
            // AttachCppRuntime will set startStatus accordingly.
            Logger::logError("%s", startStatus.ToString().c_str());
            return false;
        }

        started = true;
        startStatus = absl::OkStatus();
        return true;
    }

    // Attaches to the Middleware, but does not start it.
    // Assumes that the middleware is started in another language (e.g., C++ or Dart).
    // HAS to be called AFTER start is called in ANOTHER language.
    bool CLAID::attachCppRuntime(void* handle)
    {
        Logger::logInfo("Attach cpp runtime 1");
        if(started)
        {
            Logger::logError("CLAID middleware start was called twice!");
            return false;
        }
        Logger::logInfo("Attach cpp runtime 2 %u", handle);

        claid::MiddleWare* middleware = static_cast<claid::MiddleWare*>(handle);
        Logger::logInfo("Attach cpp runtime 2.1");

        const std::string& socketPath = middleware->getSocketPath();
        Logger::logInfo("Attach cpp runtime 2.2");

        const std::set<std::string> registeredModuleClasses = ModuleFactory::getInstance()->getRegisteredModuleClasses();

        std::map<std::string, ModuleAnnotation> moduleAnnotations;

        for(const std::string& registeredModuleClass : registeredModuleClasses)
        {
            ModuleAnnotator moduleAnnotator(registeredModuleClass);

            bool res = ModuleFactory::getInstance()->getModuleAnnotation(registeredModuleClass, moduleAnnotator);
            if(res)
            {
                moduleAnnotations[registeredModuleClass] = moduleAnnotator.getAnnotation();
            }
        }
        Logger::logInfo("Attach cpp runtime 3");

        moduleDispatcher = make_unique<DispatcherClient>(socketPath, fromModuleDispatcherQueue, toModuleDispatcherQueue, registeredModuleClasses, moduleAnnotations);
        moduleManager = make_unique<ModuleManager>(
                *moduleDispatcher, fromModuleDispatcherQueue, toModuleDispatcherQueue, middleware->getEventTracker());
        Logger::logInfo("Attach cpp runtime 4");

        absl::Status status = moduleManager->start();

        if(!status.ok())
        {
            startStatus = status;
            std::stringstream ss;
            ss << status;
            Logger::logFatal("%s", ss.str().c_str());
            return false;
        }
        Logger::logInfo("Attach cpp runtime 5");

        started = true;
        return true;
    }

    bool CLAID::shutdown()
    {
        if(handle == nullptr)
        {
            return false;
        }   
        Logger::logInfo("Stopping ModuleManager.");
        moduleManager->stop();
        Logger::logInfo("Stopping ModuleDispatcher.");
        moduleDispatcher->shutdown();

        shutdown_core(handle);
        
        moduleDispatcher = nullptr;
        moduleManager = nullptr;
          
        started = false;

        return true;
    }

    bool CLAID::isConnectedToRemoteServer() const
    {
        if(handle == nullptr)
        {
            return false;
        }   
        claid::MiddleWare* middleware = static_cast<claid::MiddleWare*>(handle);
        return middleware->isConnectedToRemoteServer();
    }

    absl::Status CLAID::getStartStatus() const
    {
        return this->startStatus;
    }

    absl::Status CLAID::getRemoteClientStatus() const
    {
        if(handle == nullptr)
        {
            return absl::UnavailableError("Cannot determine status of the RemoteDispatcherClient, because middleware is not running");
        }   

        claid::MiddleWare* middleware = static_cast<claid::MiddleWare*>(handle);
        return middleware->getRemoteClientStatus();
    }

    absl::Status CLAID::loadNewConfig(const std::string& newConfigFilePath)
    {
        if(handle == nullptr)
        {
            return absl::UnavailableError("Cannot determine status of the RemoteDispatcherClient, because middleware is not running");
        }   

        claid::MiddleWare* middleware = static_cast<claid::MiddleWare*>(handle);
        return middleware->loadNewConfig(newConfigFilePath);
    }

    RemoteFunctionHandler* CLAID::getRemoteFunctionHandler()
    {
        if(this->moduleManager == nullptr)
        {
            return nullptr;
        }
        return this->moduleManager->getRemoteFunctionHandler();
    }
}