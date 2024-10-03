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
#include "FuturesTable.hh"

using namespace claidservice;
namespace claid {

class FutureUniqueIdentifier;

class AbstractFuture 
{
    public:
        typedef std::function<void (std::shared_ptr<DataPackage>)> ThenCallback;

    private:




        std::mutex mutex;
        std::condition_variable conditionVariable;
                

        // The list used by the FutureHandler to manage its associated futures.
        // Note: FuturesTable is thread safe.
        FuturesTable& futuresTableInHandler;
        FutureUniqueIdentifier uniqueIdentifier;

        ThenCallback callback;


    
    
    protected: 

        bool finished = false;
        bool successful = false;
        bool callbackSet = false;

        std::shared_ptr<DataPackage> responsePackage;

        std::shared_ptr<DataPackage> awaitResponse();
        std::shared_ptr<DataPackage> awaitResponse(int timeoutSeconds);

        void thenUntyped(ThenCallback callback);

    public:


        AbstractFuture(FuturesTable& futuresTableInHandler, 
            FutureUniqueIdentifier uniqueIdentifier);

        void setResponse(std::shared_ptr<DataPackage> responsePackage);
        void setFailed();

        FutureUniqueIdentifier getUniqueIdentifier() const;

        bool wasExecutedSuccessfully() const;
};
}