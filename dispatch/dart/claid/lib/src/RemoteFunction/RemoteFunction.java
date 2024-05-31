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

package adamma.c4dhi.claid.RemoteFunction;

import java.util.ArrayList;
import java.util.Objects;

import adamma.c4dhi.claid.Module.ThreadSafeChannel;
import adamma.c4dhi.claid.TypeMapping.DataType;
import adamma.c4dhi.claid.TypeMapping.Mutator;
import adamma.c4dhi.claid.TypeMapping.TypeMapping;
import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.RemoteFunction.FutureHandler;

import adamma.c4dhi.claid.DataPackage;
import adamma.c4dhi.claid.ControlPackage;
import adamma.c4dhi.claid.CtrlType;
import adamma.c4dhi.claid.RemoteFunctionRequest;
import adamma.c4dhi.claid.RemoteFunctionReturn;
import adamma.c4dhi.claid.RemoteFunctionIdentifier;
import adamma.c4dhi.claid.Module.ThreadSafeChannel;

import adamma.c4dhi.claid.Runtime;

// A remote function is an RPC stub, 
// which remotely calls an RemoteFunctionRunnable in another entity (another Runtime or Module).
// Actually, this should be called RemoteFunctionStub, but might be less intutive for people not familiar with RPC terminology.
public class RemoteFunction<T> 
{
    private Class<T> returnType = null;
    private ArrayList<Class<?>> parameterTypes = null;
    
    private FutureHandler futuresHandler = null;
    private ThreadSafeChannel<DataPackage> toMiddlewareQueue = null;
    private RemoteFunctionIdentifier remoteFunctionIdentifier = null;
    
    boolean successful = false;

    String sourceModule = "";

    public RemoteFunction(
        FutureHandler futuresHandler, 
        ThreadSafeChannel<DataPackage> toMiddlewareQueue,
        RemoteFunctionIdentifier remoteFunctionIdentifier,
        Class<T> returnType, ArrayList<Class<?>> parameterTypes)
    {
        this.futuresHandler = futuresHandler;
        this.toMiddlewareQueue = toMiddlewareQueue;
        this.remoteFunctionIdentifier = remoteFunctionIdentifier;

        this.returnType = returnType;
        this.parameterTypes = parameterTypes;
    }

    public RemoteFunction(
        String sourceModule,
        FutureHandler futuresHandler, 
        ThreadSafeChannel<DataPackage> toMiddlewareQueue,
        RemoteFunctionIdentifier remoteFunctionIdentifier,
        Class<T> returnType, ArrayList<Class<?>> parameterTypes)
    {
        this.sourceModule = sourceModule;
        
        this.futuresHandler = futuresHandler;
        this.toMiddlewareQueue = toMiddlewareQueue;
        this.remoteFunctionIdentifier = remoteFunctionIdentifier;

        this.returnType = returnType;
        this.parameterTypes = parameterTypes;
    }

    public Future<T> execute(Object... parameters)
    {
        if(this.remoteFunctionIdentifier.hasModuleId() && this.remoteFunctionIdentifier.getModuleId().equals(this.sourceModule))
        {
            Logger.logError("Failed to execute RPC! Module \"" + this.sourceModule + "\" tried to call an RPC function of itself, which is not allowed.");
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
                return null;
            }
        }

        Future<T> future = this.futuresHandler.registerNewFuture(this.returnType);

        DataPackage.Builder dataPackageBuilder = DataPackage.newBuilder();
        ControlPackage.Builder controlPackageBuilder = ControlPackage.newBuilder();
        controlPackageBuilder.setCtrlType(CtrlType.CTRL_REMOTE_FUNCTION_REQUEST);
        controlPackageBuilder.setRuntime(Runtime.RUNTIME_JAVA);

        RemoteFunctionRequest remoteFunctionRequest = 
            makeRemoteFunctionRequest(future.getUniqueIdentifier().toString(), parameters);

        controlPackageBuilder.setRemoteFunctionRequest(remoteFunctionRequest);

        dataPackageBuilder.setControlVal(controlPackageBuilder.build());

        if(this.remoteFunctionIdentifier.hasModuleId())
        {
            dataPackageBuilder.setTargetModule(this.remoteFunctionIdentifier.getModuleId());
            dataPackageBuilder.setSourceModule(this.sourceModule);
        }
       

        toMiddlewareQueue.add(dataPackageBuilder.build());

        return future;

    }

    private RemoteFunctionRequest makeRemoteFunctionRequest(String futureIdentifier, Object... parameters)
    {
        RemoteFunctionRequest.Builder request = RemoteFunctionRequest.newBuilder();

        request.setRemoteFunctionIdentifier(this.remoteFunctionIdentifier);
        request.setRemoteFutureIdentifier(futureIdentifier);

        // We want to use the mutator to convert each parameter to a Blob.
        // However, Mutator expects a DataPackage to set the blob directly.
        // Hence, we create a stub data package and simply retrieve it's payload, 
        // after the mutator has converted and set the blob.
        DataPackage stubPackage = DataPackage.newBuilder().build();

        for(int i = 0; i < parameters.length; i++)
        {
            Mutator<?> mutator = TypeMapping.getMutator(new DataType(parameterTypes.get(i)));
            Object tmp = parameters[i];
            stubPackage = mutator.setPackagePayloadFromObject(stubPackage, tmp);
            request.addParameterPayloads(stubPackage.getPayload());
        }

        return request.build();
    }

  

    public String getFunctionSignature()
    {
        String returnTypeName = returnType == null ? "void" : returnType.getSimpleName();
        String functionName = "";

        boolean isRuntimeFunction = this.remoteFunctionIdentifier.hasRuntime();

        if(isRuntimeFunction)
        {
            functionName = this.remoteFunctionIdentifier.getRuntime().name() + "::" + this.remoteFunctionIdentifier.getFunctionName();
        }
        else
        {
            functionName = this.remoteFunctionIdentifier.getModuleId() + "::" + this.remoteFunctionIdentifier.getFunctionName();
        }

        String parameterNames = this.parameterTypes.size() > 0 ? this.parameterTypes.get(0).getSimpleName() : "";

        for(int i = 1; i < this.parameterTypes.size(); i++)
        {
            parameterNames += ", " + this.parameterTypes.get(i).getSimpleName();
        }

        String functionSignature = isRuntimeFunction ? "RuntimeFunction: " : "ModuleFunction: ";
        functionSignature += returnTypeName + " " + functionName + " (" + parameterNames + ")";

        return functionSignature;
    }
}
