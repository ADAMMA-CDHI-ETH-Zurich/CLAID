package adamma.c4dhi.claid.RemoteFunction;

import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicReference;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import adamma.c4dhi.claid.RemoteFunction.FutureUniqueIdentifier;

public abstract class AbstractFuture 
{
    private Boolean finished = false;
    private Object returnValue = null;

    private final Lock mutex = new ReentrantLock();
    private final Condition conditionVariable = mutex.newCondition();

    // The list used by the FutureHandler to manage its associated futures.
    // Note: FuturesList is thread safe.
    private FuturesList listOfFuturesInHandler = null;

    FutureUniqueIdentifier uniqueIdentifier = new FutureUniqueIdentifier();

    public AbstractFuture(FuturesList listOfFuturesInHandler, FutureUniqueIdentifier uniqueIdentifier)
    {
        this.listOfFuturesInHandler = listOfFuturesInHandler;
        this.uniqueIdentifier = uniqueIdentifier;
    }

    protected Object await()
    {
        lock.lock();
        try 
        {
            while (!finished) 
            {
                condition.await();
            }

            // Future is finished, remove it from the list of the handler.
            if(this.listOfFuturesInHandler != null)
            {
                // This is thread safe, as FuturesList is thread safe
                this.listOfFuturesInHandler.removeFuture(this);
            }
        } 
        catch (InterruptedException e) 
        {
            Thread.currentThread().interrupt();
        } 
        finally 
        {
            lock.unlock();
        }
        return returnValue;
    }

    public void setFinished(Object returnValue) 
    {
        lock.lock();
        try 
        {
            this.returnValue = returnValue;
            this.condition.signalAll();
        } 
        finally 
        {
            lock.unlock();
        }
    }

    public FutureUniqueIdentifier getUniqueIdentifier()
    {
        return this.uniqueIdentifier;
    }
}
