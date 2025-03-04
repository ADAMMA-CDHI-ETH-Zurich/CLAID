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

#include "dispatch/proto/claidservice.pb.h"
#include "dispatch/core/Logger/Logger.hh"

namespace claid {

template<typename T>
class RemoteFunctionRunnableResult
{
    private:
        std::shared_ptr<T> returnValue;
        RemoteFunctionStatus status;

    public:
        RemoteFunctionRunnableResult(std::shared_ptr<T> returnValue, 
            RemoteFunctionStatus status) : returnValue(returnValue), status(status)
        {
        
        }

        static RemoteFunctionRunnableResult makeSuccessfulResult(std::shared_ptr<T> returnValue)
        {
            return RemoteFunctionRunnableResult(returnValue, RemoteFunctionStatus::STATUS_OK);
        }

        static RemoteFunctionRunnableResult makeFailedResult(RemoteFunctionStatus status)
        {
            return RemoteFunctionRunnableResult(nullptr, status);
        }

        RemoteFunctionStatus getStatus()
        {
            return this->status;
        }

        std::shared_ptr<T> getReturnValue()
        {
            return this->returnValue;
        }
};
}