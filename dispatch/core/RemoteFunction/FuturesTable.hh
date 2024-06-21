/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer
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

#include "dispatch/proto/claidservice.grpc.pb.h"
#include "FutureUniqueIdentifier.hh"
#include <map>
#include <mutex>

namespace claid {

class AbstractFuture;

class FuturesTable 
{
    private:
        std::map<FutureUniqueIdentifier, std::shared_ptr<AbstractFuture>> futures;
        
        std::mutex lock;

    public:

        void addFuture(std::shared_ptr<AbstractFuture> future);

        bool removeFuture(FutureUniqueIdentifier futureIdentifier);

        std::shared_ptr<AbstractFuture> lookupFuture(FutureUniqueIdentifier uniqueIdentifier);
        
};

}
