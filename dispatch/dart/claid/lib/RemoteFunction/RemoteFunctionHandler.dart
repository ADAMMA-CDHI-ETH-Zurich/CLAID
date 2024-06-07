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
import 'RemoteFunction.dart';
import 'FutureHandler.dart';
import 'AbstractRPCCompleter.dart';
import 'FutureUniqueIdentifier.dart';

import 'package:claid/generated/claidservice.pb.dart';



class RemoteFunctionHandler 
{
    FutureHandler _futuresHandler = FutureHandler();
    StreamController<DataPackage> _outputController;

    RemoteFunctionHandler(this._outputController)
    {
    }

    RemoteFunction<T> mapRuntimeFunction<T>(Runtime runtime, String functionName, T returnType, List<dynamic> parameterDataTypes)
    {
        RemoteFunction<T> function = RemoteFunction<T>(
                this._futuresHandler, 
                this._outputController, 
                makeRemoteRuntimeFunctionIdentifier(runtime, functionName), 
                returnType, 
                parameterDataTypes);

        return function;
    }

    RemoteFunction<T> mapModuleFunction<T>(String targetModule, 
        String functionName, T returnType, List<dynamic> parameterDataTypes)
    {
        RemoteFunction<T> function = new RemoteFunction<T>(
                this._futuresHandler, 
                this._outputController, 
                makeRemoteModuleFunctionIdentifier(targetModule, functionName), 
                returnType, 
                parameterDataTypes);

        return function;
    }

    RemoteFunctionIdentifier makeRemoteRuntimeFunctionIdentifier(Runtime runtime, String functionName)
    {
        RemoteFunctionIdentifier remoteFunctionIdentifier = RemoteFunctionIdentifier();
        remoteFunctionIdentifier.functionName = functionName;
        remoteFunctionIdentifier.runtime = runtime;

        return remoteFunctionIdentifier;
    }

    RemoteFunctionIdentifier makeRemoteModuleFunctionIdentifier(String moduleId, String functionName)
    {
        RemoteFunctionIdentifier remoteFunctionIdentifier = RemoteFunctionIdentifier();
        remoteFunctionIdentifier.functionName = functionName;
        remoteFunctionIdentifier.moduleId = moduleId;

        return remoteFunctionIdentifier;
    }

    void handleResponse(DataPackage remoteFunctionResponse)
    {
        print("Handle response 1");
        if(!remoteFunctionResponse.controlVal.hasRemoteFunctionReturn())
        {
            Logger.logError("Failed to handle remote function response " + 
                remoteFunctionResponse.toString() + ". Did not find RemoteFunctionReturn data");
            return;
        }
       
        print("Handle response 2");

        RemoteFunctionReturn remoteFunctionReturn = remoteFunctionResponse.controlVal.remoteFunctionReturn;
        String futureIdentifier = remoteFunctionReturn.remoteFutureIdentifier;
        FutureUniqueIdentifier uniqueIdentifier = FutureUniqueIdentifier(futureIdentifier);
        print("Handle response 3");

        AbstractRPCCompleter? future = this._futuresHandler.lookupFuture(uniqueIdentifier);
        print("Handle response 4");

        if(future == null)
        {
            Logger.logError("Failed to forward result of remote function. Cannot find future with identifier \"" + futureIdentifier + "\".");
            return;
        }
        print("Handle response 5");

        if(remoteFunctionReturn.executionStatus != RemoteFunctionStatus.STATUS_OK)
        {
            Logger.logError("Remote function failed. Future with identifier \"" + 
                futureIdentifier + "\" failed with status \"" + remoteFunctionReturn.executionStatus.toString() + "\".");
            future.setFailed();
            return;
        }
        future.setResponse(remoteFunctionResponse);

    }
}

