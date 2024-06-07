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

import 'dart:async';
import 'package:claid/Logger/Logger.dart';
import 'package:claid/generated/claidservice.pb.dart';
import 'package:claid/module/type_mapping.dart';
import 'RemoteFunctionRunnableResult.dart';
import 'RemoteFunctionRunnable.dart';
class RemoteFunctionRunnableHandler 
{

    String _entityName;
    StreamController<DataPackage> _outputController;
    Map<String, RemoteFunctionRunnable> _registeredRunnables = {};

    RemoteFunctionRunnableHandler(this._entityName, this._outputController)
    {
        
    }

    bool addRunnable(String functionName, RemoteFunctionRunnable runnable)
    {
        if(this._registeredRunnables.containsKey(functionName))
        {
            return false;
        }

        this._registeredRunnables[functionName] = runnable;
        return true;
    }

    bool registerRunnable<T>(String functionName, Function function, T returnType, List<dynamic> parameterTypes)
    {
        if(_registeredRunnables.containsKey(functionName))
        {
            Logger.logFatal("Failed to register function \"" + functionName + "\" in Module \"" + this._entityName + "\". Function already registered before.");
            return false;
        }

        
        if(!RemoteFunctionRunnable.isDataTypeSupported(returnType))
        {
            Logger.logFatal("Failed to register function \"" + functionName + "\" in Module \"" + this._entityName  +
                "\". Return type \"" + returnType.runtimeType.toString() + "\" is no CLAID data type and hence not supported.");
            return false;
        }

        for(int i = 0; i < parameterTypes.length; i++)
        {
            if(!RemoteFunctionRunnable.isDataTypeSupported(parameterTypes[i]))
            {
                Logger.logFatal("Failed to register function \"" + functionName + "\" of entity \"" + this._entityName + ". Parameter type \"" + 
                parameterTypes[i].runtimeType.toString() + "\" is no CLAID data type and hence not supported.");
                return false;
            }
        }

        RemoteFunctionRunnable runnable = new RemoteFunctionRunnable<T>(function, returnType, parameterTypes);
        
        return this.addRunnable(functionName, runnable);
    }
    
    bool executeRemoteFunctionRunnable(DataPackage rpcRequest)
    {
        RemoteFunctionRequest request;
        if(!rpcRequest.controlVal.hasRemoteFunctionRequest())
        {
            Logger.logError("Failed to execute RPC request data package. Could not find definition of RemoteFunctionRequest.");

            DataPackage response = 
                RemoteFunctionRunnable.prepareResponsePackage(RemoteFunctionStatus.REMOTE_FUNCTION_REQUEST_INVALID, rpcRequest);

            if(response != null)
            {
                this._outputController.add(response);
            }
            
            return false;
        }

        request = rpcRequest.controlVal.remoteFunctionRequest;

        RemoteFunctionIdentifier remoteFunctionIdentifier = request.remoteFunctionIdentifier;

        String functionName = remoteFunctionIdentifier.functionName;
        if(!this._registeredRunnables.containsKey(functionName))
        {
            Logger.logError("Failed to execute RPC request. Entity \"" + this._entityName + "\" does not have a registered remote function called \"" + functionName + "\".");

            DataPackage response = 
                RemoteFunctionRunnable.prepareResponsePackage(RemoteFunctionStatus.FAILED_FUNCTION_NOT_FOUND_OR_FAILED_TO_EXECUTE, rpcRequest);
                
            if(response != null)
            {
                this._outputController.add(response);
            }
            
            return false;
        }

        RemoteFunctionRunnable runnable = this._registeredRunnables[functionName]!;
        DataPackage response = runnable.executeRemoteFunctionRequest(rpcRequest);

        if(response != null)
        {
            this._outputController.add(response);
        }

        return true;
    }

    
}
