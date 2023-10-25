package adamma.c4dhi.claid.Module;

import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.locks.ReentrantLock;

import adamma.c4dhi.claid.Logger.Logger;

public class ThreadSafeChannel<T>
{
    // BlockingQueue is thred safe: https://docs.oracle.com/javase%2F7%2Fdocs%2Fapi%2F%2F/java/util/concurrent/BlockingQueue.html
    BlockingQueue<T> queue = new LinkedBlockingQueue<>();

    public void add(T element)
    {
        queue.add(element);
    }

    public T blockingGet()
    {
        T value;
        try 
        {
            value = queue.take();
        } 
        catch (InterruptedException e) 
        {
            Logger.logError("ThreadSafeChannel error: " + e.getMessage());
            value = null;
        }
    
        return value;
    }
}
