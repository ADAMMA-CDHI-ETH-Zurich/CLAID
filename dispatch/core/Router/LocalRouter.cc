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

#include "dispatch/core/Router/LocalRouter.hh"

namespace claid {

    LocalRouter::LocalRouter(const std::string& currentHost, ModuleTable& moduleTable) : moduleTable(moduleTable), currentHost(currentHost)
    {

    }

    absl::Status LocalRouter::routePackage(std::shared_ptr<DataPackage> dataPackage)
    {
        if(dataPackage->has_control_val())
        {
            return routeControlPackage(dataPackage);
        }
        const std::string& sourceHost = dataPackage->source_host();
        const std::string& sourceModule = dataPackage->source_module();

        const std::string& targetHost = dataPackage->target_host();
        const std::string& targetModule = dataPackage->target_module();

        Logger::logInfo("LocalRouter routing package from host \"%s\" (Module \"%s\"), "
                        "destined for host \"%s\" (Module \"%s\").", sourceHost.c_str(), sourceModule.c_str(), targetHost.c_str(), targetModule.c_str());

        SharedQueue<DataPackage>* inputQueueForRuntime = this->moduleTable.lookupOutputQueue(targetModule);

        if(!inputQueueForRuntime)
        {
            return absl::InvalidArgumentError(absl::StrCat("LocalRouter: Failed to route package to local Module \"", targetModule,"\".\n"
            "Unable to get input queue of the Runtime the Module is running in. Possibly, the Runtime was not registered."));
        }
        if(!inputQueueForRuntime->is_closed())
        {
            inputQueueForRuntime->push_back(dataPackage);
        }
        else
        {
            // Might happen when shutting down or a Runtime has not yet been loaded.
            // Logger::logWarning("LocalRouter inputQueueForRuntime is null, cannot route package. Are we shutting down?");
        }
        return absl::OkStatus();
    }

    absl::Status LocalRouter::routeControlPackage(std::shared_ptr<DataPackage> package)
    {
        Logger::logInfo("LocalRouter routing control package %s", CtrlType_Name(package->control_val().ctrl_type()).c_str());
        this->moduleTable.controlPackagesQueue().push_front(package);
        return absl::OkStatus();
    }

    bool LocalRouter::canReachHost(const std::string& hostname)
    {
        return hostname == currentHost;
    }
}