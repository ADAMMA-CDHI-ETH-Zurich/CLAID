package adamma.c4dhi.claid.RemoteFunction;

import adamma.c4dhi.claid.RemoteFunctionReturn;
import adamma.c4dhi.claid.RemoteFunctionStatus;

class RemoteFunctionRunnableResult
{
    Object returnValue;
    RemoteFunctionStatus status;

    public RemoteFunctionRunnableResult(Object returnValue, RemoteFunctionStatus status)
    {
        this.status = status;
        this.returnValue = returnValue;
    }

    public static RemoteFunctionRunnableResult makeSuccessfulResult(Object returnValue)
    {
        return new RemoteFunctionRunnableResult(returnValue, RemoteFunctionStatus.STATUS_OK);
    }

    public static RemoteFunctionRunnableResult makeFailedResult(RemoteFunctionStatus status)
    {
        return new RemoteFunctionRunnableResult(null, status);
    }

    public RemoteFunctionStatus getStatus()
    {
        return this.status;
    }

    public Object getReturnValue()
    {
        return this.returnValue;
    }
}