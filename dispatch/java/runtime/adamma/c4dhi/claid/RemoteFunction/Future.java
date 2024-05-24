package adamma.c4dhi.claid.RemoteFunction;


public class Future<T> extends AbstractFuture 
{
    public T await()
    {
        Object result = super.await();
        if(result == null)
        {
            return null;
        }

        return (T) result;
    }
}
