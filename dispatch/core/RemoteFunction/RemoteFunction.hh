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

#include "dispatch/core/Logger/Logger.hh"
#include "dispatch/proto/claidservice.grpc.pb.h"
#include "AbstractMutatorHelper.hh"
#include "FutureHandler.hh"

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
        
        FutureHandler& futuresHandler;
        SharedQueue<DataPackage>& toMiddlewareQueue;
        RemoteFunctionIdentifier remoteFunctionIdentifier;

        std::vector<std::shared_ptr<AbstractMutatorHelper>> mutatorHelpers;
        
        bool successful = false;

        std::string sourceModule = "";

    public:
        template<typename... Parameters>
        RemoteFunction(
            FutureHandler& futuresHandler, 
            SharedQueue<DataPackage>& toMiddlewareQueue,
            RemoteFunctionIdentifier remoteFunctionIdentifier) : 
                futuresHandler(futuresHandler), toMiddlewareQueue(toMiddlewareQueue), remoteFunctionIdentifier(remoteFunctionIdentifier)
        {
            this->mutatorHelpers = {std::static_pointer_cast<AbstractMutatorHelper>(make_shared<TypedMutatorHelper<Parameters...>>())};
            // helpers = {std::static_pointer_cast<AbstractMutatorHelper>(std::make_shared<TypedMutatorHelper<T...>>())...};
        }

        template<typename... Parameters>
        RemoteFunction(
            std::string sourceModule,
            FutureHandler& futuresHandler, 
            SharedQueue<DataPackage>& toMiddlewareQueue,
            RemoteFunctionIdentifier remoteFunctionIdentifier) : 
                sourceModule(sourceModule), futuresHandler(futuresHandler), 
                toMiddlewareQueue(toMiddlewareQueue), remoteFunctionIdentifier(remoteFunctionIdentifier)
        {
            this->mutatorHelpers = {std::static_pointer_cast<AbstractMutatorHelper>(make_shared<TypedMutatorHelper<Parameters...>>())};
        }

        template<typename... Parameters>
        std::shared_ptr<Future<T>> execute(Parameters... params)
        {
            if(this->remoteFunctionIdentifier.hasModuleId() && this.remoteFunctionIdentifier->module_id() == this.sourceModule)
            {
                Logger.logError("Failed to execute RPC! Module \"" + this.sourceModule + 
                    "\" tried to call an RPC function of itself, which is not allowed.");
                return null;
            }

            if(parameters.length != parameterTypes.size())
            {
                Logger.logError("Failed to execute RemoteFunction (RPC stub) \"" + getFunctionSignature() + "\". Number of parameters do not match. " +
                "Function expected " + parameterTypes.size() + " parameters, but was executed with " + parameters.length);
                return null;
            }

            for(int i = 0; i < parameters.length; i++)
            {
                if(!parameters[i].getClass().equals(parameterTypes.get(i)))
                {
                    Logger.logError("Failed to execute remote function \"" + getFunctionSignature() + "\". Parameter object " + i +
                    " is of type \"" + parameters[i].getClass() + "\", but expected type \"" + parameterTypes.get(i).getSimpleName() + "\".");
                    return nullptr;
                }
            }

            std::shared_ptr<Future<T>> future = this->futuresHandler.registerNewFuture<T>();

            DataPackage dataPackage;
            ControlPackage& controlPackage = *dataPackage.mutable_control_val();

            controlPackage.setCtrlType(CtrlType::CTRL_REMOTE_FUNCTION_REQUEST);
            controlPackage.setRuntime(Runtime::RUNTIME_JAVA);

            RemoteFunctionRequest& remoteFunctionRequest = *controlPackage.mutable_remote_function_request();

            remoteFunctionRequest = makeRemoteFunctionRequest(future.getUniqueIdentifier().toString(), parameters);


            if(this->remoteFunctionIdentifier.hasModuleId())
            {
                dataPackageBuilder.set_target_module(this.remoteFunctionIdentifier.getModuleId());
                dataPackageBuilder.set_source_module(this.sourceModule);
            }
        

            toMiddlewareQueue.push_back(dataPackageBuilder.build());

            return future;

        }

        template<typename... Parameters>
        void makeRemoteFunctionRequest(std::string futureIdentifier, RemoteFunctionRequest& request, Parameters... parameters)
        {
            request.set_remote_function_identifier(this.remoteFunctionIdentifier);
            request.set_remote_future_identifier(futureIdentifier);

            // We want to use the mutator to convert each parameter to a Blob.
            // However, Mutator expects a DataPackage to set the blob directly.
            // Hence, we create a stub data package and simply retrieve it's payload, 
            // after the mutator has converted and set the blob.
            DataPackage stubPackage;

            const int parametersLength = sizeof...(parameters);
            for(int i = 0; i < parametersLength; i++)
            {
                std::shared_ptr<AbstractMutatorHelper> helper = this->mutatorHelpers[i];
                helper->setPackagePayload(stubPackage, st);

                setParameterPayloads<0, Parameters...>(request, parameters...);
            }

        }

        template<int C, typename U, typename... Us>
        void setParameterPayloads(RemoteFunctionRequest& request, U& parameter, Us&... rest)
        {
            DataPackage stubPackage;

            std::shared_ptr<AbstractMutatorHelper> helper = this->mutatorHelpers[i];
            helper->setPackagePayload(stubPackage, parameter);
            (*request.add_parameter_payloads()) = stubPackage.payload();

            setParameterPayloads<C + 1, Us...>(request, rest...);
        }

        template<int C>
        void setParameterPayloads(RemoteFunctionRequest& request)
        {
            // Base case
        }
    

        std::string getFunctionSignature()
        {
            std::string returnTypeName = typeid(T).name();
            std::string functionName = "";

            bool isRuntimeFunction = this.remoteFunctionIdentifier.hasRuntime();

            if(isRuntimeFunction)
            {
                functionName = this.remoteFunctionIdentifier.runtime().name() + "::" + this.remoteFunctionIdentifier.function_name();
            }
            else
            {
                functionName = this.remoteFunctionIdentifier.module_id() + "::" + this.remoteFunctionIdentifier.function_name();
            }

            std::string parameterNames = this.parameterTypes.size() > 0 ? this->mutatorHelpers[0].getDataTypeName() : "";

            for(int i = 1; i < this.parameterTypes.size(); i++)
            {
                parameterNames += ", " + this->mutatorHelpers[i].getDataTypeName();
            }

            std::string functionSignature = isRuntimeFunction ? "RuntimeFunction: " : "ModuleFunction: ";
            functionSignature += returnTypeName + " " + functionName + " (" + parameterNames + ")";

            return functionSignature;
        }
};
}
