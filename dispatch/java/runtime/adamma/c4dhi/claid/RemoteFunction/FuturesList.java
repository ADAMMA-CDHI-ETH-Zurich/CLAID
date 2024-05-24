package adamma.c4dhi.claid.RemoteFunction;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.concurrent.locks.ReentrantLock;

public class FuturesList 
{
    private Map<String, AbstractFuture> futures = new HashMap<>();
    private final ReentrantLock lock = new ReentrantLock();

    public void addFuture(AbstractFuture future)
    {
        lock.lock();
        futures.add(future.getUniqueIdentifier().toString(), future);
        lock.unlock();
    }

    public boolean removeFuture(AbstractFuture future)
    {
        lock.lock();
        boolean wasFound = futures.remove(future);
        lock.unlock();
        return wasFound;
    }
    
}
