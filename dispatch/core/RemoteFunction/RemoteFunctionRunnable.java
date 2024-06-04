package adamma.c4dhi.claid.RemoteFunction;


// The RemoteFunctionRunnable is the counterpart to the RemoteFunction.
// The RemoteFunctionRunnable is the actual function, which get's called remotely.
// E.g., it could be a function registered inside a Module. 

import java.lang.reflect.Method;

import java.util.ArrayList;

import adamma.c4dhi.claid.RemoteFunctionRequest;
import adamma.c4dhi.claid.RemoteFunctionReturn;
import adamma.c4dhi.claid.RemoteFunctionStatus;

import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.TypeMapping.DataType;
import adamma.c4dhi.claid.TypeMapping.Mutator;
import adamma.c4dhi.claid.TypeMapping.TypeMapping;

import adamma.c4dhi.claid.DataPackage;
import adamma.c4dhi.claid.ControlPackage;
import adamma.c4dhi.claid.CtrlType;
import adamma.c4dhi.claid.Blob;
import adamma.c4dhi.claid.Runtime;

import adamma.c4dhi.claid.RemoteFunctionIdentifier;

import adamma.c4dhi.claid.RemoteFunction.RemoteFunctionRunnableResult;
import adamma.c4dhi.claid.RemoteFunction.RemoteFunctionRunnable;

import com.google.protobuf.GeneratedMessageV3;

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

        int payloadsSize = executionRequest.getParameterPayloadsCount();

        if(payloadsSize!= this.parameterTypes.size())
        {
            Logger.logError("Failed to execute RemoteFunctionRunnable \"" + getFunctionSignature(remoteFunctionIdentifier, executionRequest) + "\". Number of parameters do not match. " +
            "Function expected " + parameterTypes.size() + " parameters, but was executed with " + payloadsSize);

            status = RemoteFunctionRunnableResult.makeFailedResult(RemoteFunctionStatus.FAILED_INVALID_NUMBER_OF_PARAMETERS);
            return makeRPCResponsePackage(status, rpcRequest);
        }

        ArrayList<Object> parameters = new ArrayList<>();
        for(int i = 0; i < payloadsSize; i++)
        {
            Class<?> dataType = this.parameterTypes.get(i);
            Mutator<?> mutator = TypeMapping.getMutator(new DataType(dataType));

            DataPackage.Builder tmpPackage = DataPackage.newBuilder();
            tmpPackage.setPayload(executionRequest.getParameterPayloads(i));

            Object data = mutator.getPackagePayload(tmpPackage.build());
            parameters.add(data);

            if(!parameters.get(i).getClass().equals(this.parameterTypes.get(i)))
            {
                Logger.logError("Failed to execute RemoteFunctionRunnable \"" + getFunctionSignature(remoteFunctionIdentifier, executionRequest) + "\". Parameter object " + i +
                " is of type \"" + parameters.get(i).getClass() + "\", but expected type \"" + parameterTypes.get(i).getSimpleName() + "\".");
                status = RemoteFunctionRunnableResult.makeFailedResult(RemoteFunctionStatus.FAILED_MISMATCHING_PARAMETERS);
                return makeRPCResponsePackage(status, rpcRequest);
            }
        }

        status = executeRemoteFunctionRequest(object, parameters);

        return makeRPCResponsePackage(status, rpcRequest);
    }

    RemoteFunctionRunnableResult executeRemoteFunctionRequest(Object object, ArrayList<Object> parameters)
    {
        try 
        {        
            Method method = RemoteFunctionRunnable.lookupMethod(object, functionName, parameterTypes);
            if(method == null)
            {
                return RemoteFunctionRunnableResult.makeFailedResult(RemoteFunctionStatus.FAILED_FUNCTION_NOT_FOUND_OR_FAILED_TO_EXECUTE);
            }

            // Convert the ArrayList to an array of Objects
            Object[] paramsArray = parameters.toArray();

            // Invoke the method on the instance with the parameters
            Object result = method.invoke(object, paramsArray);

            return RemoteFunctionRunnableResult.makeSuccessfulResult(result);
        } 
        catch (Exception e) 
        {
            e.printStackTrace();
            Logger.logError("Failed to execute RemoteFunctionRunnable. Got exception \"" + e.getMessage() + "\".");
            return RemoteFunctionRunnableResult.makeFailedResult(RemoteFunctionStatus.FAILED_FUNCTION_NOT_FOUND_OR_FAILED_TO_EXECUTE);
        }
    }

    public static Method lookupMethod(Object object, String functionName, ArrayList<Class<?>> parameterTypes)
    {
        Logger.logError("dbg 1");
        try 
        {        
            Class<?> myClass = object.getClass();
            Logger.logError("dbg 2");

            Class<?>[] parameterTypesArray = new Class<?>[parameterTypes.size()];
            parameterTypesArray = parameterTypes.toArray(parameterTypesArray);
            Logger.logError("dbg 3");

            Method method = myClass.getMethod(functionName, parameterTypesArray);        
            Logger.logError("dbg 4 " + method);

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
        Logger.logError("Dbg 0 " + parameterTypes.size());
        return RemoteFunctionRunnable.lookupMethod(object, functionName, parameterTypes) != null;
    }

    public static boolean isDataTypeSupported(Class<?> type)
    {
        Mutator<?> mutator = TypeMapping.getMutator(new DataType(type));
        
        return mutator != null;
    }

    public String getFunctionSignature(RemoteFunctionIdentifier remoteFunctionIdentifier, RemoteFunctionRequest RemoteFunctionRequest)
    {
        String returnTypeName = returnType == null ? "void" : returnType.getSimpleName();

        boolean isRuntimeFunction = remoteFunctionIdentifier.hasRuntime();
    
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


        return remoteFunctionReturn.build();
    }

    DataPackage makeRPCResponsePackage(RemoteFunctionRunnableResult result, DataPackage rpcRequest)
    {
        RemoteFunctionRequest executionRequest = rpcRequest.getControlVal().getRemoteFunctionRequest();

        DataPackage.Builder responseBuilder = DataPackage.newBuilder();

        responseBuilder.setSourceModule(rpcRequest.getTargetModule());
        responseBuilder.setTargetModule(rpcRequest.getSourceModule());


        ControlPackage.Builder ctrlPackage = ControlPackage.newBuilder();
        ctrlPackage.setCtrlType(CtrlType.CTRL_REMOTE_FUNCTION_RESPONSE);
        ctrlPackage.setRemoteFunctionReturn(makeRemoteFunctionReturn(result, executionRequest));

        ctrlPackage.setRuntime(Runtime.RUNTIME_JAVA);


        responseBuilder.setControlVal(ctrlPackage.build());

        DataPackage responsePackage = responseBuilder.build();

        Object returnValue = result.getReturnValue();
        if(returnValue != null)
        {
            Mutator<?> mutator = TypeMapping.getMutator(new DataType(this.returnType));
            responsePackage = mutator.setPackagePayloadFromObject(responsePackage, returnValue);
        }

        return responsePackage;
    }
}
