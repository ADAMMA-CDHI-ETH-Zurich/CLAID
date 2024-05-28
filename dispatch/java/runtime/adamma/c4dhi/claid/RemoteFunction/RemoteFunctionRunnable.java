


// The RemoteFunctionRunnable is the counterpart to the RemoteFunction.
// The RemoteFunctionRunnable is the actual function, which get's called remotely.
// E.g., it could be a function registered inside a Module. 

import java.lang.reflect.Method;
import java.rmi.Remote;
import java.sql.Blob;
import java.util.ArrayList;

import adamma.c4dhi.claid.RemoteFunctionRequest;
import adamma.c4dhi.claid.RemoteFunctionReturn;
import adamma.c4dhi.claid.RemoteFunctionExecutionStatus;
import adamma.c4dhi.claid.RemoteFunctionRunnableResult;

import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.TypeMapping.DataType;
import adamma.c4dhi.claid.TypeMapping.Mutator;
import adamma.c4dhi.claid.TypeMapping.TypeMapping;

import adamma.c4dhi.claid.ControlPackage;
import adamma.c4dhi.claid.CtrlType;

public class RemoteFunctionRunnable 
{
    private String functionName;
    private Class<?> returnType;
    private ArrayList<Class<?>> parameterTypes;

    RemoteFunctionRunnable(String functionName, Class<?> returnType, ArrayList<Class<?>> parameterTypes)
    {
        this.functionName = functionName;
        this.returnType = returnType;
        this.parameterTypes = parameterTypes;
    }

    DataPackage executeRemoteFunctionRequest(Object object, DataPackage rpcRequest)
    {
        RemoteFunctionRunnableResult status;

        RemoteFunctionRequest executionRequest = rpcRequest.getControlVal().getRemoteFunctionRequest();

        RemoteFunctionIdentifier remoteFunctionIdentifier = executionRequest.getRemoteFunctionIdentifier();

        ArrayList<Blob> payloads = request.getParameterPayloads();

        if(payloads.size() != this.parameterTypes.size())
        {
            Logger.logError("Failed to execute RemoteFunctionRunnable \"" + getFunctionSignature(remoteFunctionIdentifier) + "\". Number of parameters do not match. " +
            "Function expected " + parameterTypes.size() + " parameters, but was executed with " + parameters.size());

            status = RemoteFunctionRunnableResult.makeFailedResult(RemoteFunctionExecutionStatus.FAILED_INVALID_NUMBER_OF_PARAMETERS);
            return makeRemoteFunctionReturn(status);
        }

        ArrayList<Object> parameters = new ArrayList<>();
        for(int i = 0; i < this.parameterTypes.size(); i++)
        {
            Class<?> dataType = this.parameterTypes.get(i);
            Mutator<?> mutator = TypeMapping.getMutator(new DataType(dataType));

            DataPackage tmpPackage = DataPackage.newBuilder().build();
            tmpPackage = tmpPackage.setPayload(payloads.get(i));

            Object data = mutator.getPackagePayload(tmpPackage.build());
            parameters.add(data);
        }

        status = executeRemoteFunctionRequest(object, parameters);

        return makeRPCResponsePackage(status, rpcRequest);
    }

    RemoteFunctionRunnableResult executeRemoteFunctionRequest(Object object, ArrayList<Object> parameters)
    {
        try 
        {        
            for(int i = 0; i < this.parameterTypes.size(); i++)
            {
                if(!parameters.get(i).getClass().equals(this.parameterTypes.get(i)))
                {
                    Logger.logError("Failed to execute RemoteFunctionRunnable \"" + getFunctionSignature() + "\". Parameter object " + i +
                    " is of type \"" + parameters.get(i).getClass() + "\", but expected type \"" + parameterTypes.get(i).getSimpleName() + "\".");
                }
                return RemoteFunctionRunnableResult.makeFailedResult(RemoteFunctionExecutionStatus.FAILED_MISMATCHING_PARAMETERS);

            }
            Class<?> myClass = object.getClass();

            Method method = RemoteFunctionRunnable.lookupMethod(object, functionName, parameterTypes);
            if(method == null)
            {
                return RemoteFunctionRunnableResult.makeFailedResult(RemoteFunctionExecutionStatus.FAILED_FUNCTION_NOT_FOUND_OR_FAILED_TO_EXECUTE);
            }

            // Convert the ArrayList to an array of Objects
            Object[] paramsArray = parameters.toArray();

            // Invoke the method on the instance with the parameters
            Obj result = method.invoke(myClassInstance, paramsArray);

            return RemoteFunctionRunnableResult.makeSuccessfulResult(result);
        } 
        catch (Exception e) 
        {
            e.printStackTrace();
            Logger.logError("Failed to execute RemoteFunctionRunnable. Got exception \"" + e.getMessage() + "\".");
            return RemoteFunctionRunnableResult.makeFailedResult(RemoteFunctionExecutionStatus.FAILED_FUNCTION_NOT_FOUND_OR_FAILED_TO_EXECUTE);
        }
    }

    public static Method lookupMethod(Object object, String functionName, ArrayList<Class<?>> parameterTypes)
    {
        try 
        {        
            Class<?> myClass = object.getClass();

            Method method = myClass.getMethod(functionName, parameterTypes.toArray());        

            return method;
        } 
        catch (Exception e) 
        {
            e.printStackTrace();
            Logger.logError("Failed to lookup function Method \"" + functionName + "\" of object of type \"" + object.getClass().getName() + "\". Got exception \"" + e.getMessage() + "\".");
            return null;
        }
    }

    public static boolean doesFunctionExist(Object object, String functionName, ArrayList<Class<?>> parameterTypes)
    {
        return RemoteFunctionRunnable.lookupMethod(object, functionName, parameterTypes) != null;
    }

    public static boolean isDataTypeSupported(Class<?> type)
    {
        Mutator<?> mutator = TypeMapping.getMutator(new DataType(type));
        
        return mutator != null && GeneratedMessageV3.class.isAssignableFrom(type);
    }

    public String getFunctionSignature(RemoteFunctionIdentifier remoteFunctionIdentifier, RemoteFunctionRequest RemoteFunctionRequest)
    {
        String returnTypeName = returnType == null ? "void" : returnType.getSimpleName();

        boolean isRuntimeFunction = remoteFunctionIdentifier.getFunctionType().name().equals("runtime");

        String parameterNames = this.parameterTypes.size() > 0 ? this.parameterTypes.get(0).getSimpleName() : "";

        for(int i = 1; i < this.parameterTypes.size(); i++)
        {
            parameterNames += ", " + this.parameterTypes.get(i).getSimpleName();
        }

        String functionSignature = isRuntimeFunction ? "RuntimeFunction: " : "ModuleFunction: ";
        functionSignature += returnTypeName + " " + functionName + " (" + parameterNames + ")";

        return functionSignature;
    }

    RemoteFunctionReturn makeRemoteFunctionReturn(RemoteFunctionRunnableResult result, RemoteFunctionRequest executionRequest)
    {
        RemoteFunctionReturn.Builder remoteFunctionReturn = RemoteFunctionReturn.newBuilder();
        RemoteFunctionIdentifier remoteFunctionIdentifier = executionRequest.getRemoteFunctionIdentifier();

        remoteFunctionReturn.setExecutionStatus(result.getStatus());
        remoteFunctionReturn.setRemoteFunctionIdentifier(remoteFunctionIdentifier);
        remoteFunctionReturn.setRemoteFutureIdentifier(executionRequest.getRemoteFutureIdentifier());


        return remoteFunctionReturn;
    }

    DataPackage makeRPCResponsePackage(RemoteFunctionRunnableResult result, DataPackage rpcRequest)
    {
        RemoteFunctionRequest executionRequest = rpcRequest.getControlVal().getRemoteFunctionRequest();

        DataPackage.Builder reponseBuilder = DataPackage.newBuilder();

        reponseBuilder.setSourceModule(rpcRequest.getTargetModule());
        responseBuilder.setTargetModule(rpcRequest.getSourceModule());


        ControlPackage.Builder ctrlPackage = ControlPackage.newBuilder();
        ctrlPackage.setCtrlType(CtrlType.CTRL_REMOTE_FUNCTION_RESPONSE);
        ctrlPackage.setRemoteFunctionReturn(makeRemoteFunctionReturn(result, executionRequest));

        if(executionRequest.hasRuntime())
        {
            ctrlPackage.setRuntime(executionRequest.getRuntime());
        }

        responseBuilder.setControlVal(ctrlPackage.build());

        DataPackage responsePackage = responseBuilder.build();

        Object returnValue = result.getReturnValue();
        if(returnValue != null)
        {
            Mutator<?> mutator = TypeMapping.getMutator(new DataType(this.returnType));
            responsePackage = mutator.setPackagePayload(responsePackage, returnValue);
        }

        return responsePackage;
    }
}
