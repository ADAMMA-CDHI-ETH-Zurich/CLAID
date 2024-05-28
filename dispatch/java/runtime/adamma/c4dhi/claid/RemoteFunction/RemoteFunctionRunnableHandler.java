package adamma.c4dhi.claid.RemoteFunction;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

import adamma.c4dhi.claid.RemoteFunctionRunnable;

public class RemoteFunctionRunnableHandler 
{

    private String entityName;
    private Map<String, RemoteFunctionRunnable> registeredRunnables = new HashMap<>();

    RemoteFunctionHandler(String entityName)
    {
        this.entityName = entityName;
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

        if(registeredRunnables.containsKey(functionName))
        {
            moduleError("Failed to register function \"" + functionName + "\" in Module \"" + this.entityName + "\" (\"" + this.getType() + "\". Function already registered before.");
            return false;
        }

        if(!RemoteFunctionRunnable.doesFunctionExist(object, functionName, returnType, parameterList))
        {
            moduleError("Failed to register function \"" + functionName + "\" in Module \"" + this.entityName + "\" (\"" + this.getType() + "\". Function not found or invalid parameters.");
            return false;
        }

        if(!RemoteFunctionRunnable.isDataTypeSupported(returnType))
        {
            moduleError("Failed to register function \"" + functionName + "\" in Module \"" + this.entityName + "\" (\"" + this.getType() +
                "\". Return type \"" + returnType.getName() + "\" is no CLAID data type and hence not supported.");
            return false;
        }

        for(int i = 0; i < parameters.length; i++)
        {
            if(!RemoteFunctionRunnable.isDataTypeSupported(parameters[i]))
            {
                Logger.logError("Failed to register function \"" + functionName + "\" of entity \"" + this.entityName + ". Parameter type \"" + 
                    parameters[i].getName() + "\" is no CLAID data type and hence not supported.");
                return false;
            }
        }

        RemoteFunctionRunnable runnable = new RemoteFunctionRunnable(functionName, returnType, parameterList);
        
        return this.addRunnable(functionName, runnable);
    }
    
    public DataPackage executeRemoteFunctionRunnable(DataPackage rpcRequest)
    {
        RemoteFunctionRequest request;
        if(!rpcRequest.getControlVal().hasRemoteFunctionRequest())
        {
            moduleError("Failed to execute RPC request data package. Could not find definition of RemoteFunctionRequest.");
            return null;
        }

        request = rpcRequest.getControlVal().getRemoteFunctionRequest();

        RemoteFunctionIdentifier remoteFunctionIdentifier = request.getRemoteFunctionIdentifier();

        String functionName = remoteFunctionIdentifier.getFunctionName();
        if(!this.registeredRunnables.containsKey(functionName))
        {
            Logger.logError("Failed to execute RPC request. Entity \"" + this.entityName + "\" does not have a registered remote function called \"" + functionName + "\".");
            return null;
        }

        RemoteFunctionRunnable runnable = this.registeredRunnables.get(functionName);
        DataPackage response = runnable.executeRemoteFunctionRequest(this, rpcRequest);

        return response;
    }
}
