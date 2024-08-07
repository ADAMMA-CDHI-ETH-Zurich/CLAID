package adamma.c4dhi.claid.RemoteFunction;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

import adamma.c4dhi.claid.RemoteFunction.RemoteFunctionRunnable;
import adamma.c4dhi.claid.DataPackage;
import adamma.c4dhi.claid.RemoteFunctionStatus;

import adamma.c4dhi.claid.RemoteFunctionRequest;
import adamma.c4dhi.claid.RemoteFunctionIdentifier;
import adamma.c4dhi.claid.Module.ThreadSafeChannel;


import adamma.c4dhi.claid.Logger.Logger;

public class RemoteFunctionRunnableHandler 
{

    private String entityName;
    private ThreadSafeChannel<DataPackage> toMiddlewareQueue;
    private Map<String, RemoteFunctionRunnable> registeredRunnables = new HashMap<>();

    public RemoteFunctionRunnableHandler(String entityName, ThreadSafeChannel<DataPackage> toMiddlewareQueue)
    {
        this.entityName = entityName;
        this.toMiddlewareQueue = toMiddlewareQueue;
    }

    public boolean addRunnable(String functionName, RemoteFunctionRunnable runnable)
    {
        if(this.registeredRunnables.containsKey(functionName))
        {
            return false;
        }

        this.registeredRunnables.put(functionName, runnable);
        return true;
    }

    public boolean registerRunnable(Object object, String functionName, Class<?> returnType, Class<?>... parameterTypes)
    {
        ArrayList<Class<?>> parameterList = new ArrayList<>(Arrays.asList(parameterTypes));
        Logger.logError("-1 " + parameterTypes.length + " " + parameterList.size());

        if(registeredRunnables.containsKey(functionName))
        {
            Logger.logFatal("Failed to register function \"" + functionName + "\" in Module \"" + this.entityName + "\". Function already registered before.");
            return false;
        }

        if(!RemoteFunctionRunnable.doesFunctionExist(object, functionName, parameterList))
        {
            Logger.logFatal("Failed to register function \"" + functionName + "\" in Module \"" + this.entityName + "\". Function not found or invalid parameters.");
            return false;
        }

        if(!RemoteFunctionRunnable.isDataTypeSupported(returnType))
        {
            Logger.logFatal("Failed to register function \"" + functionName + "\" in Module \"" + this.entityName  +
                "\". Return type \"" + returnType.getName() + "\" is no CLAID data type and hence not supported.");
            return false;
        }

        for(int i = 0; i < parameterTypes.length; i++)
        {
            if(!RemoteFunctionRunnable.isDataTypeSupported(parameterTypes[i]))
            {
                Logger.logFatal("Failed to register function \"" + functionName + "\" of entity \"" + this.entityName + ". Parameter type \"" + 
                parameterTypes[i].getName() + "\" is no CLAID data type and hence not supported.");
                return false;
            }
        }

        RemoteFunctionRunnable runnable = new RemoteFunctionRunnable(object, functionName, returnType, parameterList);
        
        return this.addRunnable(functionName, runnable);
    }
    
    public boolean executeRemoteFunctionRunnable(DataPackage rpcRequest)
    {
        RemoteFunctionRequest request;
        if(!rpcRequest.getControlVal().hasRemoteFunctionRequest())
        {
            Logger.logError("Failed to execute RPC request data package. Could not find definition of RemoteFunctionRequest.");
            DataPackage response = 
                RemoteFunctionRunnable.makeRPCResponsePackage(
                    RemoteFunctionRunnableResult.makeFailedResult(
                        RemoteFunctionStatus.REMOTE_FUNCTION_REQUEST_INVALID),
                    rpcRequest, null);

            if(response != null)
            {
                this.toMiddlewareQueue.add(response);
            }
            return false;
        }

        request = rpcRequest.getControlVal().getRemoteFunctionRequest();

        RemoteFunctionIdentifier remoteFunctionIdentifier = request.getRemoteFunctionIdentifier();

        String functionName = remoteFunctionIdentifier.getFunctionName();
        if(!this.registeredRunnables.containsKey(functionName))
        {
            Logger.logError("Failed to execute RPC request. Entity \"" + this.entityName + "\" does not have a registered remote function called \"" + functionName + "\".");
            DataPackage response = 
                RemoteFunctionRunnable.makeRPCResponsePackage(
                    RemoteFunctionRunnableResult.makeFailedResult(
                        RemoteFunctionStatus.FAILED_FUNCTION_NOT_FOUND_OR_FAILED_TO_EXECUTE),
                    rpcRequest, null);

            if(response != null)
            {
                this.toMiddlewareQueue.add(response);
            }
            return false;
        }
        else
        {
            RemoteFunctionRunnable runnable = this.registeredRunnables.get(functionName);
            DataPackage response = runnable.executeRemoteFunctionRequest(rpcRequest);

            if(response != null)
            {
                this.toMiddlewareQueue.add(response);
            }
        }
        
        

        return true;
    }
}
