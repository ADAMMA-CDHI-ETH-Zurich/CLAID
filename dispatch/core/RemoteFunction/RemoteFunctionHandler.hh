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

using namespace claidservice;

namespace claid {
    class RemoteFunctionHandler 
{
    private:

        FutureHandler futuresHandler;
        SharedQueue<DataPackage>& toMiddlewareQueue;

        
        RemoteFunctionIdentifier makeRemoteFunctionIdentifier(Runtime runtime, std::string functionName)
        {
            RemoteFunctionIdentifier remoteFunctionIdentifier;
            remoteFunctionIdentifier.set_function_name(functionName);
            remoteFunctionIdentifier.set_runtime(runtime);

            return remoteFunctionIdentifier;
        }

        RemoteFunctionIdentifier makeRemoteFunctionIdentifier(std::string moduleId, std::string functionName)
        {
            RemoteFunctionIdentifier remoteFunctionIdentifier;
            remoteFunctionIdentifier.set_function_name(functionName);
            remoteFunctionIdentifier.set_module_id(moduleId);

            return remoteFunctionIdentifier;
        }

       

    public:
        RemoteFunctionHandler(SharedQueue<DataPackage>& toMiddlewareQueue) : toMiddlewareQueue(toMiddlewareQueue)
        {
        }

        template<typename Return, typename... Parameters>
        RemoteFunction<Return> mapRuntimeFunction(Runtime runtime, std::string functionName)
        {
            RemoteFunction<Return> function = makeRemoteFunction<Return, Parameters...>(
                    &this->futuresHandler, 
                    &this->toMiddlewareQueue, 
                    makeRemoteFunctionIdentifier(runtime, functionName));

            return function;
        }

        template<typename Return, typename... Parameters>
        RemoteFunction<Return> mapModuleFunction(std::string sourceModule, std::string targetModule, std::string functionName)
        {
            RemoteFunction<Return> function = makeRemoteFunction<Return, Parameters...>(
                    sourceModule,
                    &this->futuresHandler, 
                    &this->toMiddlewareQueue, 
                    makeRemoteFunctionIdentifier(targetModule, functionName));

            return function;
        }

         void handleResponse(std::shared_ptr<DataPackage> remoteFunctionResponse)
        {
            if(!remoteFunctionResponse->control_val().has_remote_function_return())
            {
                Logger::logError("Failed to handle remote function response \"%s\". Did not find RemoteFunctionReturn data", 
                messageToString(*remoteFunctionResponse).c_str());
                return;
            }
        

            const RemoteFunctionReturn& remoteFunctionReturn = remoteFunctionResponse->control_val().remote_function_return();

            std::string futureIdentifier = remoteFunctionReturn.remote_future_identifier();
            FutureUniqueIdentifier uniqueIdentifier = FutureUniqueIdentifier::fromString(futureIdentifier);

            std::shared_ptr<AbstractFuture> future = this->futuresHandler.lookupFuture(uniqueIdentifier);

            if(future == nullptr)
            {
                Logger::logError("Failed to forward result of remote function. Cannot find future with identifier \"%s\".", futureIdentifier.c_str());
                return;
            }

            if(remoteFunctionReturn.execution_status() != RemoteFunctionStatus::STATUS_OK)
            {
                Logger::logError("Remote function failed. Future with identifier \"%s\" failed with status \"%s\".",
                futureIdentifier.c_str(), RemoteFunctionStatus_Name(remoteFunctionReturn.execution_status()).c_str());

                future->setFailed();
                return;
            }
            future->setResponse(remoteFunctionResponse);

        }
};

}


