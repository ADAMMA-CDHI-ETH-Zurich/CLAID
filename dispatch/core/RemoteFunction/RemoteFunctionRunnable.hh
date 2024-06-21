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
#include "dispatch/core/Logger/Logger.hh"
#include "FutureHandler.hh"
#include "dispatch/core/shared_queue.hh"
#include "RemoteFunction.hh"
#include "dispatch/core/proto_util.hh"
#include "dispatch/core/Utilities/VariadicTemplateHelpers.hh"
#include "AbstractRemoteFunctionRunnable.hh"
#include "RemoteFunctionRunnableResult.hh"

using namespace claidservice;

namespace claid {

template<typename Return, typename... Parameters>
class RemoteFunctionRunnable : public AbstractRemoteFunctionRunnable
{
    private:
        std::string functionName;
        
        // Binds function and object!
        // Works for class and non-class methods.
        typedef std::function<Return (Parameters...)> FunctionType;
        FunctionType function;

        std::vector<std::shared_ptr<AbstractMutatorHelper>> mutatorHelpers;

        

        

        template<int C, typename U, typename... Us>
        void extractParameters(const RemoteFunctionRequest& request, std::tuple<Parameters...>& tuple)
        {
            DataPackage tmpPackage;
            *tmpPackage.mutable_payload() = request.parameter_payloads(C);

            Mutator<U> mutator = TypeMapping::getMutator<U>();
            
            mutator.getPackagePayload(tmpPackage, std::get<C>(tuple));

            extractParameters<C + 1, Us...>(request, tuple);
        }

        template<int C>
        void extractParameters(const RemoteFunctionRequest& request, std::tuple<Parameters...>& tuple)
        {

        }

        template<typename U = Return>
        typename std::enable_if<std::is_same<void, U>::value, RemoteFunctionRunnableResult<U>>::type
        executeRemoteFunctionRequestFromTuple(std::tuple<Parameters...>& tuple)
        {
            VariadicTemplateHelpers::applyTupleToFunction(tuple, function);

            return RemoteFunctionRunnableResult<Return>::makeSuccessfulResult(nullptr);        
        }

        template<typename U = Return>
        typename std::enable_if<!std::is_same<void, U>::value, RemoteFunctionRunnableResult<U>>::type
        executeRemoteFunctionRequestFromTuple(std::tuple<Parameters...>& tuple)
        {
            Return result = VariadicTemplateHelpers::applyTupleToFunction(tuple, function);

            return RemoteFunctionRunnableResult<U>::makeSuccessfulResult(std::make_shared<Return>(result));        
        }

    public:
        RemoteFunctionRunnable(std::string functionName, 
            FunctionType function) : functionName(functionName), function(function)
        {
            this->mutatorHelpers = {std::static_pointer_cast<AbstractMutatorHelper>(std::make_shared<TypedMutatorHelper<Parameters>>())...};
        }

        std::shared_ptr<DataPackage> executeRemoteFunctionRequest(std::shared_ptr<DataPackage> rpcRequest) override final
        {
            RemoteFunctionRequest executionRequest = rpcRequest->control_val().remote_function_request();

            RemoteFunctionIdentifier remoteFunctionIdentifier = executionRequest.remote_function_identifier();

            int payloadsSize = executionRequest.parameter_payloads_size();

            int parameterSize = sizeof...(Parameters);
            if(payloadsSize != parameterSize)
            {
                Logger::logError("Failed to execute RemoteFunctionRunnable \"%s\". Number of parameters do not match. Function expected %d parameters, but was executed with %d",
                getFunctionSignature(remoteFunctionIdentifier).c_str(), parameterSize, payloadsSize);

                RemoteFunctionRunnableResult<Return> status = 
                    RemoteFunctionRunnableResult<Return>::makeFailedResult(RemoteFunctionStatus::FAILED_INVALID_NUMBER_OF_PARAMETERS);
                return makeRPCResponsePackage(status, rpcRequest);
            }

            std::tuple<Parameters...> parameters;

            extractParameters<0, Parameters...>(rpcRequest->control_val().remote_function_request(), parameters);

        
            RemoteFunctionRunnableResult<Return> status
                = executeRemoteFunctionRequestFromTuple(parameters);

            return makeRPCResponsePackage(status, rpcRequest);
        }

        

        template<typename U>
        static typename std::enable_if<!std::is_same<U, void>::value, bool>::type
        isDataTypeSupported()
        {
            // Will fail at compile time if U is not supported
            Mutator<U> mutator = TypeMapping::getMutator<U>();
            
            return true;
        }

        template<typename U>
        static typename std::enable_if<std::is_same<U, void>::value, bool>::type
        isDataTypeSupported()
        {
            return true;
        }

        std::string getFunctionSignature(RemoteFunctionIdentifier remoteFunctionIdentifier)
        {
            std::string returnTypeName = typeid(Return).name();
            std::string functionName = "";

            bool isRuntimeFunction = remoteFunctionIdentifier.has_runtime();

            if(isRuntimeFunction)
            {
                functionName = Runtime_Name(remoteFunctionIdentifier.runtime()) + "::" + remoteFunctionIdentifier.function_name();
            }
            else
            {
                functionName = remoteFunctionIdentifier.module_id() + "::" + remoteFunctionIdentifier.function_name();
            }

            std::string parameterNames = this->mutatorHelpers.size() > 0 ? this->mutatorHelpers[0]->getTypeName() : "";

            for(int i = 1; i < this->mutatorHelpers.size(); i++)
            {
                parameterNames += ", " + this->mutatorHelpers[i]->getTypeName();
            }

            std::string functionSignature = isRuntimeFunction ? "RuntimeFunction: " : "ModuleFunction: ";
            functionSignature += returnTypeName + " " + functionName + " (" + parameterNames + ")";

            return functionSignature;
        }

        RemoteFunctionReturn makeRemoteFunctionReturn(RemoteFunctionRunnableResult<Return> result, RemoteFunctionRequest executionRequest)
        {
            RemoteFunctionReturn remoteFunctionReturn;
            RemoteFunctionIdentifier remoteFunctionIdentifier = executionRequest.remote_function_identifier();

            remoteFunctionReturn.set_execution_status(result.getStatus());
            *(remoteFunctionReturn.mutable_remote_function_identifier()) = remoteFunctionIdentifier;
            remoteFunctionReturn.set_remote_future_identifier(executionRequest.remote_future_identifier());


            return remoteFunctionReturn;
        }

        std::shared_ptr<DataPackage> 
            makeRPCResponsePackage(RemoteFunctionRunnableResult<Return>& result, std::shared_ptr<DataPackage> rpcRequest)
        {
            const RemoteFunctionRequest& executionRequest = rpcRequest->control_val().remote_function_request();

            std::shared_ptr<DataPackage> responsePackage = std::make_shared<DataPackage>();

            responsePackage->set_source_module(rpcRequest->target_module());
            responsePackage->set_target_module(rpcRequest->source_module());


            ControlPackage& ctrlPackage = *(responsePackage->mutable_control_val());
            ctrlPackage.set_ctrl_type(CtrlType::CTRL_REMOTE_FUNCTION_RESPONSE);
            (*ctrlPackage.mutable_remote_function_return()) = makeRemoteFunctionReturn(result, executionRequest);

            // Send back to the runtime where the rpcRequest came from.
            ctrlPackage.set_runtime(rpcRequest->control_val().runtime());

            setReturnPackagePayload<Return>(responsePackage, result);
            

            return responsePackage;
        }

        template<typename U>
        typename std::enable_if<!std::is_same<U, void>::value>::type
        setReturnPackagePayload(std::shared_ptr<DataPackage> package, RemoteFunctionRunnableResult<U>& result)
        {
            std::shared_ptr<U> returnValue = result.getReturnValue();
            if(returnValue != nullptr)
            {
                Mutator<U> mutator = TypeMapping::getMutator<U>();
                mutator.setPackagePayload(*package, *returnValue);
            }
        }

        template<typename U>
        typename std::enable_if<std::is_same<U, void>::value>::type
        setReturnPackagePayload(std::shared_ptr<DataPackage> package, RemoteFunctionRunnableResult<U>& result)
        {
            
        }
};

}