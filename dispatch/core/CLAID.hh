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

#include "dispatch/core/Configuration/Configuration.hh"
#include "dispatch/core/Router/MasterRouter.hh"
#include "dispatch/core/Module/ModuleManager.hh"
#include "dispatch/core/local_dispatching.hh"
#include "dispatch/core/middleware.hh"

#include "dispatch/core/capi.h"
namespace claid
{
    class CLAID
    {

    private:
  
        std::unique_ptr<DispatcherClient> moduleDispatcher;
        std::unique_ptr<ModuleManager> moduleManager;


        SharedQueue<DataPackage> fromModuleDispatcherQueue;
        SharedQueue<DataPackage> toModuleDispatcherQueue;

        bool started = false;
        void* handle;

        absl::Status startStatus;

    public:


        // Starts the middleware and attaches to it.
        bool start(const std::string& socketPath, const std::string& configFilePath, const std::string& hostId, 
            const std::string& userId, const std::string& deviceId, std::string commonDataPath);
        
        bool shutdown();

        // Attaches to the Middleware, but does not start it.
        // Assumes that the middleware is started in another language (e.g., C++ or Dart).
        // HAS to be called AFTER start is called in ANOTHER language.
        bool attachCppRuntime(void* handle);

        bool isConnectedToRemoteServer() const;
        absl::Status getStartStatus() const;
        absl::Status getRemoteClientStatus() const;
        
        absl::Status loadNewConfig(const std::string& newConfigFilePath);
    };

}
