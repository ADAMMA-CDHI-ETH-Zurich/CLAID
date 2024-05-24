


// The RemoteFunctionRunnable is the counterpart to the RemoteFunction.
// The RemoteFunctionRunnable is the actual function, which get's called remotely.
// E.g., it could be a function registered inside a Module. 

import java.lang.reflect.Method;
import java.rmi.Remote;
import java.util.ArrayList;

import adamma.c4dhi.claid.RemoteFunctionRequest;
import adamma.c4dhi.claid.RemoteFunctionReturn;

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

    class RemoteFunctionRunnableResult
    {
        Object returnValue = true;
        boolean successful = false;

        public RemoteFunctionRunnableResult(Object returnValue, boolean successful)
        {
            this.returnValue = returnValue;
            this.successful = successful;
        }

        public static RemoteFunctionRunnableResult makeSuccessfulResult(Object returnValue)
        {
            return new RemoteFunctionRunnableResult(returnValue, true);
        }

        public static RemoteFunctionRunnableResult makeFailedResult()
        {
            return new RemoteFunctionRunnableResult(null, false);
        }
    }

    RemoteFunctionReturn executeRemoteFunctionRequest(RemoteFunctionRequest request)
    {

    }

    RemoteFunctionRunnableResult execute(Object object, ArrayList<Object> parameters)
    {
        try {
            // Create an instance of MyClass
            MyClass myClassInstance = new MyClass();

            // The parameters to pass to myFunction

            // Get the Class object associated with MyClass
            Class<?> myClass = object.getClass();

            if(parameters.size() != this.parameterTypes.size())
            {
                Logger.logError("Failed to execute RemoteFunctionRunnable \"" + getFunctionSignature() + "\". Number of parameters do not match. " +
                "Function expected " + parameterTypes.size() + " parameters, but was executed with " + parameters.size());

                return RemoteFunctionRunnableResult.makeFailedResult();
            }

            for(int i = 0; i < this.parameterTypes.size(); i++)
            {
                if(!parameters.get(i).getClass().equals(this.parameterTypes.get(i)))
                {
                    Logger.logError("Failed to execute RemoteFunctionRunnable \"" + getFunctionSignature() + "\". Parameter object " + i +
                    " is of type \"" + parameters.get(i).getClass() + "\", but expected type \"" + parameterTypes.get(i).getSimpleName() + "\".");
                }
                return RemoteFunctionRunnableResult.makeFailedResult();

            }

            // Get the Method object for myFunction with specified parameter types
            Method method = myClass.getMethod(this.functionName, this.parameterTypes.toArray());

            // Convert the ArrayList to an array of Objects
            Object[] paramsArray = parameters.toArray();

            // Invoke the method on the instance with the parameters
            Obj result = method.invoke(myClassInstance, paramsArray);

            return RemoteFunctionRunnableResult.makeSuccessfulResult(result);

        } catch (Exception e) {
            e.printStackTrace();
            Logger.logError("Failed to execute RemoteFunctionRunnable. Got exception \"" + e.getMessage() + "\".");
            return RemoteFunctionRunnableResult.makeFailedResult();
        }
    }

    public String getFunctionSignature()
    {
        String returnTypeName = returnType == null ? "void" : returnType.getSimpleName();

        boolean isRuntimeFunction = this.remoteFunctionIdentifier.getFunctionType().name().equals("runtime");


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
