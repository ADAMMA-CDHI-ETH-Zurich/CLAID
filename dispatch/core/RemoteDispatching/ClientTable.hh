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

#include "dispatch/proto/claidservice.pb.h"
#include "dispatch/core/shared_queue.hh"

using claidservice::DataPackage;

namespace claid {

    // Analog to the ModuleTable for the LocalRouter and the HostUserTable for the ServerRouter,
    // the ClientTable enables the ClientRouter to access the queue to route packages via a connection to an external server.
    
    class ClientTable
    {
        private:
            SharedQueue<DataPackage> toRemoteClientQueue;
            SharedQueue<DataPackage> fromRemoteClientQueue;

        public:
            ClientTable();

            SharedQueue<DataPackage>& getFromRemoteClientQueue();
            SharedQueue<DataPackage>& getToRemoteClientQueue();

    };
}