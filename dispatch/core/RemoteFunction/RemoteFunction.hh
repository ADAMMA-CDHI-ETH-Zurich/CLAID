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

#include <string>

#include "dispatch/proto/claidservice.pb.h"
#include "dispatch/core/Logger/Logger.hh"
#include "AbstractMutatorHelper.hh"
#include "TypedMutatorHelper.hh"
#include "FutureHandler.hh"
#include "dispatch/core/proto_util.hh"
#include <functional>
// A remote function is an RPC stub, 
// which remotely calls an RemoteFunctionRunnable in another entity (another Runtime or Module).
// Actually, this should be called RemoteFunctionStub, but might be less intutive for people not familiar with RPC terminology.

using namespace claidservice;

namespace claid { 
template<typename T>
class RemoteFunction
{
    private:

        // ArrayList<Class<?>> parameterTypes = null;
        
        FutureHandler* futuresHandler = nullptr;
        SharedQueue<DataPackage>* toMiddlewareQueue = nullptr;
        RemoteFunctionIdentifier remoteFunctionIdentifier;

        std::vector<std::shared_ptr<AbstractMutatorHelper>> mutatorHelpers;
        
        bool successful = false;


        bool valid = false;



    public:
        RemoteFunction()
        {
            this->valid = false;
        }

        RemoteFunction(
            FutureHandler* futuresHandler, 
            SharedQueue<DataPackage>* toMiddlewareQueue,
            RemoteFunctionIdentifier remoteFunctionIdentifier,
            std::vector<std::shared_ptr<AbstractMutatorHelper>> mutatorHelpers) : 
                futuresHandler(futuresHandler), toMiddlewareQueue(toMiddlewareQueue), 
                remoteFunctionIdentifier(remoteFunctionIdentifier), mutatorHelpers(mutatorHelpers)
        {
            // this->mutatorHelpers = {std::static_pointer_cast<AbstractMutatorHelper>(std::make_shared<TypedMutatorHelper<Parameters...>>())};
            // helpers = {std::static_pointer_cast<AbstractMutatorHelper>(std::make_shared<TypedMutatorHelper<T...>>())...};
            this->valid = true;
        }

        

        static RemoteFunction InvalidRemoteFunction()
        {
            // Using private constructor to create invalid function
            return RemoteFunction();
        }


        template<typename... Parameters>
        std::shared_ptr<Future<T>> execute(Parameters... params)
        {
            if(!this->valid)
            {
                Logger::logError("Failed to execute RemoteFunction (RPC stub) \"%s\". Function is not valid.", getFunctionSignature().c_str());      
                return nullptr;
            }

            size_t parametersLength = sizeof...(params);
            if(parametersLength != mutatorHelpers.size())
            {
                Logger::logError("Failed to execute RemoteFunction (RPC stub) \"%s\". "
                "Number of parameters do not match. Function expected %d parameters, but was executed with %d",
                getFunctionSignature().c_str(), mutatorHelpers.size(), parametersLength);
                return nullptr;
            }

            if(!checkParameterTypes<0, Parameters...>(params...))
            {
                return nullptr;
            }
            

            std::shared_ptr<Future<T>> future = this->futuresHandler->template registerNewFuture<T>();

            std::shared_ptr<DataPackage> dataPackage = std::make_shared<DataPackage>();
            ControlPackage& controlPackage = *(dataPackage->mutable_control_val());

            controlPackage.set_ctrl_type(CtrlType::CTRL_REMOTE_FUNCTION_REQUEST);
            controlPackage.set_runtime(Runtime::RUNTIME_CPP);

            RemoteFunctionRequest& remoteFunctionRequest = *controlPackage.mutable_remote_function_request();

            makeRemoteFunctionRequest(future->getUniqueIdentifier().toString(), remoteFunctionRequest, params...);


            if(this->remoteFunctionIdentifier.has_module_id())
            {
                dataPackage->set_target_module(this->remoteFunctionIdentifier.module_id());
            }
        

            toMiddlewareQueue->push_back(dataPackage);

            return future;

        }

        template<typename... Parameters>
        void makeRemoteFunctionRequest(std::string futureIdentifier, RemoteFunctionRequest& request, Parameters... parameters)
        {
            *(request.mutable_remote_function_identifier()) = this->remoteFunctionIdentifier;
            request.set_remote_future_identifier(futureIdentifier);

            // We want to use the mutator to convert each parameter to a Blob.
            // However, Mutator expects a DataPackage to set the blob directly.
            // Hence, we create a stub data package and simply retrieve it's payload, 
            // after the mutator has converted and set the blob.
            DataPackage stubPackage;


            setParameterPayloads<0, Parameters...>(request, parameters...);
          

        }

        template<int C, typename U, typename... Us>
        void setParameterPayloads(RemoteFunctionRequest& request, U& parameter, Us&... rest)
        {
            DataPackage stubPackage;

            std::shared_ptr<AbstractMutatorHelper> helper = this->mutatorHelpers[C];
            helper->setPackagePayload(stubPackage, parameter);
            
            Logger::logInfo("Stub package %s", messageToString(stubPackage).c_str());
            (*request.add_parameter_payloads()) = stubPackage.payload();

            setParameterPayloads<C + 1, Us...>(request, rest...);
        }

        template<int C>
        void setParameterPayloads(RemoteFunctionRequest& request)
        {
            // Base case
        }

        template<int C, typename U, typename... Us>
        bool checkParameterTypes(U& parameter, Us&... rest)
        {
            if(C >= mutatorHelpers.size())
            {
                return false;
            }

            if(!mutatorHelpers[C]->template isSameType<U>())
            {
                Logger::logError("Failed to execute remote function \"%s\". Parameter object %d is of type \"%s\", but expected type \"%s\".",
                getFunctionSignature().c_str(), C, TypeMapping::getMutator<U>().getMessageTypeName().c_str(), mutatorHelpers[C]->getTypeName().c_str());
                return false;
            }
            return checkParameterTypes<C + 1, Us...>(rest...);
        }

        template<int C>
        bool checkParameterTypes()
        {
            return true;
        }

        std::string getFunctionSignature()
        {
            std::string returnTypeName = typeid(T).name();
            std::string functionName = "";

            bool isRuntimeFunction = this->remoteFunctionIdentifier.has_runtime();

            if(isRuntimeFunction)
            {
                functionName = Runtime_Name(this->remoteFunctionIdentifier.runtime()) + "::" + this->remoteFunctionIdentifier.function_name();
            }
            else
            {
                functionName = this->remoteFunctionIdentifier.module_id() + "::" + this->remoteFunctionIdentifier.function_name();
            }

            std::string parameterNames = this->mutatorHelpers.size() > 0 ? this->mutatorHelpers[0]->getTypeName() : "";

            for(size_t i = 1; i < this->mutatorHelpers.size(); i++)
            {
                parameterNames += ", " + this->mutatorHelpers[i]->getTypeName();
            }

            std::string functionSignature = isRuntimeFunction ? "RuntimeFunction: " : "ModuleFunction: ";
            functionSignature += returnTypeName + " " + functionName + " (" + parameterNames + ")";

            return functionSignature;
        }
};


template<typename Return, typename... Parameters>
RemoteFunction<Return> makeRemoteFunction(FutureHandler* futuresHandler, SharedQueue<DataPackage>* queue, RemoteFunctionIdentifier identifier)
{
    std::vector<std::shared_ptr<AbstractMutatorHelper>> mutatorHelpers = 
        {std::static_pointer_cast<AbstractMutatorHelper>(std::make_shared<TypedMutatorHelper<Parameters>>())...};

    return RemoteFunction<Return>(futuresHandler, queue, identifier, mutatorHelpers);
}

template<typename Return, typename... Parameters>
RemoteFunction<Return> makeRemoteFunction(
        std::string moduleId, FutureHandler* futuresHandler, 
        SharedQueue<DataPackage>* queue, RemoteFunctionIdentifier identifier)
{
    std::vector<std::shared_ptr<AbstractMutatorHelper>> mutatorHelpers = {std::static_pointer_cast<AbstractMutatorHelper>(std::make_shared<TypedMutatorHelper<Parameters>>())...};

    return RemoteFunction<Return>(moduleId, futuresHandler, queue, identifier, mutatorHelpers);
}

}
