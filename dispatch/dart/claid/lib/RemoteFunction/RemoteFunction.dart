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
import 'FutureHandler.dart';
import 'RPCCompleter.dart';
import 'package:claid/generated/claidservice.pb.dart';
import 'package:claid/logger/Logger.dart';
import 'package:claid/module/type_mapping.dart';
// A remote function is an RPC stub, 
// which remotely calls an RemoteFunctionRunnable in another entity (another Runtime or Module).
// Actually, this should be called RemoteFunctionStub, but might be less intutive for people not familiar with RPC terminology.
class RemoteFunction<T> 
{
    T _returnType;
    List<dynamic> _parameterTypes;
    
    FutureHandler _futuresHandler;
    StreamController<DataPackage> _outputController;
    RemoteFunctionIdentifier _remoteFunctionIdentifier;
    
    bool successful = false;


    RemoteFunction(
        this._futuresHandler, 
        this._outputController,
        this._remoteFunctionIdentifier,
        this._returnType, this._parameterTypes)
    {

    }

   

    Future<T?> execute(List<dynamic> parameters) async
    {
        if(parameters.length != _parameterTypes.length)
        {
            Logger.logError("Failed to execute RemoteFunction (RPC stub) \"" + getFunctionSignature() + "\". Number of parameters do not match. " +
            "Function expected " + _parameterTypes.length.toString() + " parameters, but was executed with " + parameters.length.toString());
            return null;
        }

        for(int i = 0; i < parameters.length; i++)
        {
            if(parameters[i].runtimeType != this._parameterTypes[i].runtimeType)
            {
                Logger.logError("Failed to execute remote function \"" + getFunctionSignature() + "\". Parameter object " + i.toString() +
                 " is of type \"" + parameters[i].runtimeType.toString() + "\", but expected type \"" + _parameterTypes[i].runtimeType.toString() + "\".");
                return null;
            }
        }

        RPCCompleter<T> future = this._futuresHandler.registerNewFuture(this._returnType);

        DataPackage dataPackage = DataPackage();
        ControlPackage controlPackage = ControlPackage();
        controlPackage.ctrlType = CtrlType.CTRL_REMOTE_FUNCTION_REQUEST;
        controlPackage.runtime = Runtime.RUNTIME_DART;

        RemoteFunctionRequest remoteFunctionRequest = 
            makeRemoteFunctionRequest(future.getUniqueIdentifier().toString(), parameters);

        controlPackage.remoteFunctionRequest = remoteFunctionRequest;

        dataPackage.controlVal = controlPackage;

        if(this._remoteFunctionIdentifier.hasModuleId())
        {
            dataPackage.targetModule = this._remoteFunctionIdentifier.moduleId;
        }
       

        _outputController.add(dataPackage);

        return future.getResponse();

    }

    RemoteFunctionRequest makeRemoteFunctionRequest(String futureIdentifier, List<dynamic> parameters)
    {
        RemoteFunctionRequest request = new RemoteFunctionRequest();

        request.remoteFunctionIdentifier = this._remoteFunctionIdentifier;
        request.remoteFutureIdentifier = futureIdentifier;

        // We want to use the mutator to convert each parameter to a Blob.
        // However, Mutator expects a DataPackage to set the blob directly.
        // Hence, we create a stub data package and simply retrieve it's payload, 
        // after the mutator has converted and set the blob.
        DataPackage stubPackage = new DataPackage();

        for(int i = 0; i < parameters.length; i++)
        {
            Mutator mutator = TypeMapping().getMutator(_parameterTypes[i]);
            
            mutator.setter(stubPackage, parameters[i]);
            request.parameterPayloads.add(stubPackage.payload);
        }

        return request;
    }

  

    String getFunctionSignature()
    {
        String returnTypeName = (T == _getType<void>()) ? "void" : _returnType.runtimeType.toString();
        String functionName = "";

        bool isRuntimeFunction = this._remoteFunctionIdentifier.hasRuntime();

        if(isRuntimeFunction)
        {
            functionName = this._remoteFunctionIdentifier.runtime.toString() + "::" + this._remoteFunctionIdentifier.functionName;
        }
        else
        {
            functionName = this._remoteFunctionIdentifier.moduleId + "::" + this._remoteFunctionIdentifier.functionName;
        }

        String parameterNames = this._parameterTypes.length > 0 ? this._parameterTypes[0].runtimeType.toString() : "";

        for(int i = 1; i < this._parameterTypes.length; i++)
        {
            parameterNames += ", " + this._parameterTypes[i].runtimeType.toString();
        }

        String functionSignature = isRuntimeFunction ? "RuntimeFunction: " : "ModuleFunction: ";
        functionSignature += returnTypeName + " " + functionName + " (" + parameterNames + ")";

        return functionSignature;
    }

    Type _getType<T>() => T;
}
