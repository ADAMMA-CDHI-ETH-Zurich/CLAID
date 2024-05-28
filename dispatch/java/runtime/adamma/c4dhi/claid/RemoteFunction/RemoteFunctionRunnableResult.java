package adamma.c4dhi.claid.RemoteFunction;

class RemoteFunctionRunnableResult
{
    Object returnValue;
    RemoteFunctionExecutionStatus status;

    public RemoteFunctionRunnableResult(Object returnValue, RemoteFunctionExecutionStatus status)
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

    public RemoteFunctionExecutionStatus getStatus()
    {
        return this.status;
    }

    public Object getReturnValue()
    {
        return this.returnValue;
    }
}