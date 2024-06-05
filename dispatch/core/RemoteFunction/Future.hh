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

#include "AbstractFuture.hh"
#include "dispatch/core/Module/TypeMapping/TypeMapping.hh"

namespace claid {

template<typename T>
class Future : public AbstractFuture 
{
public:
    typedef std::function<void (const T&)> TypedThenCallback;

    bool typedCallbackSet = false;
    TypedThenCallback callbackFunction;

    Future(FuturesTable& futuresTableInHandler, 
        FutureUniqueIdentifier uniqueIdentifier)  : AbstractFuture(futuresTableInHandler, uniqueIdentifier)
    {
    }

    template<typename U = T>
    typename std::enable_if<std::is_same<U, void>::value, void>::type await()
    {
        std::shared_ptr<DataPackage> responsePackage = this->awaitResponse();
    }


    template<typename U = T>
    typename std::enable_if<!std::is_same<U, void>::value,U>::type await()
    {
        std::shared_ptr<DataPackage> responsePackage = this->awaitResponse();
        if(responsePackage == nullptr || !this->wasExecutedSuccessfully())
        {
            return T();
        }

        Mutator<T> mutator = TypeMapping::getMutator<T>();

        T t = mutator.getPackagePayload(responsePackage);

        return t;
    }
    

    void then(TypedThenCallback callbackFunction)
    {
        this->callbackFunction = callbackFunction;
        this->typedCallbackSet = true;

        this->thenUntyped(std::bind(&Future::callback, this));
    }

    void callback(std::shared_ptr<DataPackage> data)
    {
        if(!this->typedCallbackSet)
        {
            return;
        }
        
        Mutator<T> mutator = TypeMapping::getMutator<T>();

        T t = mutator.getPackagePayload(responsePackage);
        this->typedCallbackSet(t);
    }
};

}