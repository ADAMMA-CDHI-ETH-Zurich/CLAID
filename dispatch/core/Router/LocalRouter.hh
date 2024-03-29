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

#include "dispatch/core/Router/Router.hh"
#include "dispatch/core/module_table.hh"
#include "dispatch/core/Logger/Logger.hh"

using claidservice::CtrlType;

namespace claid
{
    // The LocalRouter is invoked from the MasterRouter, if a package is targeted
    // for a Module on the current host (this host).
    // The LocalRouter then routes the package to the correct RuntimeDispatcher via the corresponding queues.

    class LocalRouter final : public Router
    {
        private:
            ModuleTable& moduleTable;
            const std::string currentHost;

            absl::Status routeControlPackage(std::shared_ptr<DataPackage> package);

        public:
        
            LocalRouter(const std::string& currentHost, ModuleTable& moduleTable);

            absl::Status routePackage(std::shared_ptr<DataPackage> dataPackage) override final;

            bool canReachHost(const std::string& hostname) override final;

    };
}