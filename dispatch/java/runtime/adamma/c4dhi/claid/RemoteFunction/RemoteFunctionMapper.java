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

import adamma.c4dhi.claid.Module.ThreadSafeChannel;
import adamma.c4dhi.claid.Module.Scheduling.ConsumerRunnable;
import adamma.c4dhi.claid.RemoteFunction.RemoteFunction;

public class RemoteFunctionMapper 
{
    private FuturesHandler futuresHandler = new FuturesHandler();
    private ThreadSafeChannel<DataPackage> toMiddlewareQueue;

    public RemoteFunctionMapper(ThreadSafeChannel<DataPackage> toMiddlewareQueue)
    {
        this.toMiddlewareQueue = toMiddlewareQueue;
    }

    public<T> RemoteFunction<T> mapRuntimeFunction(Runtime runtime, String functionName, Class<T> returnType, Class<?>... parameterDataTypes)
    {
        RemoteFunction<T> function = new RemoteFunction<>(
                this.futuresHandler, 
                this.toMiddlewareQueue, 
                makeRemoteFunctionIdentifier(runtime, functionName), 
                returnType, 
                parameterDataTypes);

        return function;
    }

    public <T> RemoteFunction<T> mapModuleFunction(String moduleId, String functionName, Class<T> returnType, Class<?>... parameterDataTypes)
    {
        RemoteFunction<T> function = new RemoteFunction<>(
                this.futuresHandler, 
                this.toMiddlewareQueue, 
                makeRemoteFunctionIdentifier(moduleId, functionName), 
                returnType, 
                parameterDataTypes);

        return function;
    }

    private RemoteFunctionIdentifier makeRemoteFunctionIdentifier(Runtime runtime, String functionName)
    {
        RemoteFunctionIdentifier.Builder remoteFunctionIdentifier = RemoteFunctionIdentifier.newBuilder();
        remoteFunctionIdentifier.setFunctionName(functionName);
        remoteFunctionIdentifier.setRuntime(runtime);

        return remoteFunctionIdentifier.build();
    }

    private RemoteFunctionIdentifier makeRemoteFunctionIdentifier(String moduleId, String functionName)
    {
        RemoteFunctionIdentifier.Builder remoteFunctionIdentifier = RemoteFunctionIdentifier.newBuilder();
        remoteFunctionIdentifier.setFunctionName(functionName);
        remoteFunctionIdentifier.setModuleId(runtime);

        return remoteFunctionIdentifier.build();
    }
}

