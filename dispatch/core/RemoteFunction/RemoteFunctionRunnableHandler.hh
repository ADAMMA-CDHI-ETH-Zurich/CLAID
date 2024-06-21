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
#include "AbstractRemoteFunctionRunnable.hh"
#include "RemoteFunctionRunnable.hh"

using namespace claidservice;

namespace claid {

class RemoteFunctionRunnableHandler 
{

public:
     RemoteFunctionRunnableHandler(const std::string& entityName,
        SharedQueue<DataPackage>& toMiddlewareQueue);
        
    std::string entityName;

    SharedQueue<DataPackage>& toMiddlewareQueue;
    std::map<std::string, std::shared_ptr<AbstractRemoteFunctionRunnable>> registeredRunnables;

   

    template<typename Class, typename Return, typename... Parameters>
    bool registerRunnable(std::string functionName, Return (Class::*f)(Parameters...), Class* obj)
    {
        auto it = this->registeredRunnables.find(functionName);

        if(it != this->registeredRunnables.end())
        {
            Logger::logFatal("Failed to register function \"%s\" in Module \"%s\". Function already registered before.",
            functionName.c_str(), this->entityName.c_str());
            return false;
        }

        

        if(!RemoteFunctionRunnable<Return>::template isDataTypeSupported<Return>())
        {
            Logger::logFatal("Failed to register function \"%s\" in Module \"%s\". Return type \"%s\" is no CLAID data type and hence not supported.", functionName.c_str(), this->entityName.c_str(), typeid(Return).name());
            return false;
        }

        // Not necessary to check this. When creating the RemoteFunctionRunnable, it creates mutators for each parameter type, 
        // which will fail at compile time if a certain parameter is not supported.
        // for(int i = 0; i < parameterTypes.length; i++)
        // {
        //     if(!RemoteFunctionRunnable.isDataTypeSupported(parameterTypes[i]))
        //     {
        //         Logger.logFatal("Failed to register function \"" + functionName + "\" of entity \"" + this->entityName + ". Parameter type \"" + 
        //         parameterTypes[i].getName() + "\" is no CLAID data type and hence not supported.");
        //         return false;
        //     }
        // }

        
        std::function<Return (Parameters...)> function = 
            VariadicTemplateHelpers::bind_with_variadic_placeholders_and_return<Class, Return, Parameters...>(f, obj);

        std::shared_ptr<RemoteFunctionRunnable<Return, Parameters...>> runnable = 
            std::make_shared<RemoteFunctionRunnable<Return, Parameters...>>(functionName, function);
        
        return this->addRunnable(functionName, std::static_pointer_cast<AbstractRemoteFunctionRunnable>(runnable));
    }
    
    bool executeRemoteFunctionRunnable(std::shared_ptr<DataPackage> rpcRequest);

    private:
        bool addRunnable(std::string functionName, std::shared_ptr<AbstractRemoteFunctionRunnable> runnable);
};


}

