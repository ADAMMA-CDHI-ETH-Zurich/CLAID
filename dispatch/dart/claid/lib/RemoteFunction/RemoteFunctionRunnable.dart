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
import 'package:claid/logger/Logger.dart';
import 'package:claid/generated/claidservice.pb.dart';
import 'package:claid/module/type_mapping.dart';
import 'RemoteFunctionRunnableResult.dart';


class RemoteFunctionRunnable<T>
{
  Function _function;
  T _returnType;
  List<dynamic> _parameterTypes; 

  RemoteFunctionRunnable(
      this._function, this._returnType, this._parameterTypes) 
  {

  }

  DataPackage executeRemoteFunctionRequest(DataPackage rpcRequest) 
  {
    RemoteFunctionRunnableResult<T> status;

    RemoteFunctionRequest executionRequest =
        rpcRequest.controlVal.remoteFunctionRequest;

    RemoteFunctionIdentifier remoteFunctionIdentifier =
        executionRequest.remoteFunctionIdentifier;

    int payloadsSize = executionRequest.parameterPayloads.length;

    if (payloadsSize != this._parameterTypes.length) 
    {
      Logger.logError(
          'Failed to execute RemoteFunctionRunnable "${getFunctionSignature(remoteFunctionIdentifier, executionRequest)}". Number of parameters do not match. Function expected ${this._parameterTypes.length} parameters, but was executed with $payloadsSize');

      status = RemoteFunctionRunnableResult.makeFailedResult(
          RemoteFunctionStatus.FAILED_INVALID_NUMBER_OF_PARAMETERS);
      return makeRPCResponsePackage(status, rpcRequest);
    }

    List<dynamic> parameters = [];
    for (int i = 0; i < payloadsSize; i++) {
      Mutator mutator = TypeMapping().getMutator(this._parameterTypes[i]);

      DataPackage tmpPackage = DataPackage();
      tmpPackage.payload = executionRequest.parameterPayloads[i];

      
      dynamic data = mutator.getter(tmpPackage);
      parameters.add(data);

      if (parameters[i].runtimeType != this._parameterTypes[i].runtimeType) 
      {
        Logger.logError(
            'Failed to execute RemoteFunctionRunnable "${getFunctionSignature(remoteFunctionIdentifier, executionRequest)}". Parameter object $i is of type "${parameters[i].runtimeType}", but expected type "${this._parameterTypes[i]}".');
        status = RemoteFunctionRunnableResult.makeFailedResult(
            RemoteFunctionStatus.FAILED_MISMATCHING_PARAMETERS);
        return makeRPCResponsePackage(status, rpcRequest);
      }
    }

    status = executeFunction(parameters);

    return makeRPCResponsePackage(status, rpcRequest);
  }

  RemoteFunctionRunnableResult<T> executeFunction(List<dynamic> parameters) 
  {
      T result = Function.apply(this._function, parameters);
      return RemoteFunctionRunnableResult.makeSuccessfulResult(result);
  }

  String getFunctionSignature(
      RemoteFunctionIdentifier remoteFunctionIdentifier,
      RemoteFunctionRequest remoteFunctionRequest) 
    {
    String returnTypeName = T == _getType<void>() ? 'void' : this._returnType.runtimeType.toString();

    bool isRuntimeFunction = remoteFunctionIdentifier.hasRuntime();

    String parameterNames =
        this._parameterTypes.isNotEmpty ? this._parameterTypes[0].runtimeType.toString() : '';

    for (int i = 1; i < this._parameterTypes.length; i++) {
      parameterNames += ', ' + this._parameterTypes[i].runtimeType.toString();
    }

  String functionName = remoteFunctionIdentifier.functionName;

    String functionSignature =
        isRuntimeFunction ? 'RuntimeFunction: ' : 'ModuleFunction: ';
    functionSignature +=
        '$returnTypeName $functionName ($parameterNames)';

    return functionSignature;
  }

  static RemoteFunctionReturn makeRemoteFunctionReturn(
      RemoteFunctionStatus status, RemoteFunctionRequest executionRequest) {
    RemoteFunctionReturn remoteFunctionReturn = RemoteFunctionReturn();

    RemoteFunctionIdentifier remoteFunctionIdentifier = executionRequest.remoteFunctionIdentifier;

    remoteFunctionReturn.executionStatus = status;
    remoteFunctionReturn.remoteFunctionIdentifier = remoteFunctionIdentifier;
    remoteFunctionReturn.remoteFutureIdentifier =
        executionRequest.remoteFutureIdentifier;

    return remoteFunctionReturn;
  }
  
  static DataPackage prepareResponsePackage(RemoteFunctionStatus status, DataPackage rpcRequest)
  {
    RemoteFunctionRequest executionRequest =
        rpcRequest.controlVal.remoteFunctionRequest;

    DataPackage responsePackage = DataPackage();

    responsePackage.sourceModule = rpcRequest.targetModule;
    responsePackage.targetModule = rpcRequest.sourceModule;

    ControlPackage ctrlPackage = ControlPackage();
    ctrlPackage.ctrlType = CtrlType.CTRL_REMOTE_FUNCTION_RESPONSE;
    ctrlPackage.remoteFunctionReturn =
        RemoteFunctionRunnable.makeRemoteFunctionReturn(status, executionRequest);

    // Send back to the runtime that made the RPC request.
    ctrlPackage.runtime = rpcRequest.controlVal.runtime;

    responsePackage.controlVal = ctrlPackage;

    return responsePackage;
  }

  DataPackage makeRPCResponsePackage(
      RemoteFunctionRunnableResult<T> result, DataPackage rpcRequest) {
    
    DataPackage responsePackage = prepareResponsePackage(result.getStatus(), rpcRequest);

    T? returnValue = result.getReturnValue();
    if(returnValue != null)
    {
      print("Return type " + this._returnType.runtimeType.toString());
        Mutator<T> mutator = TypeMapping().getMutator(this._returnType);
        mutator.setter(responsePackage, returnValue);
    }

    return responsePackage;
  }

    static bool isDataTypeSupported(dynamic dataTypeExample)
    {
      // Leads to compile error or exception if data type is not supported
      TypeMapping().getMutator(dataTypeExample);
      return true;
    }

    Type _getType<T>() => T;

}
