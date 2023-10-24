package adamma.c4dhi.claid.Module;

import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.locks.ReentrantLock;

import adamma.c4dhi.claid.Logger.Logger;

public class ThreadSafeChannel<T>
{
    BlockingQueue<T> queue = new LinkedBlockingQueue();
    private final ReentrantLock mutex = new ReentrantLock();

    public void add(T element)
    {
        mutex.lock();
        queue.add(element);
        mutex.unlock();;
    }

    public T blockingGet()
    {
        mutex.lock();
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
        mutex.unlock();
        return value;
    }
}
