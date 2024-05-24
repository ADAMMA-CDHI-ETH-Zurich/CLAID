package adamma.c4dhi.claid.RemoteFunction;

import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class FutureUniqueIdentifier 
{
    private static final Lock mutex = new ReentrantLock();
    private static long currentId = 0;

    private String identifier;

    public static FutureUniqueIdentifier makeUniqueIdentifier()
    {
        mutex.lock();
        int idCopy = currentId;
        currentId++;
        mutex.unlock();

        String identifier = "CLAID_JAVA_" + idCopy + "_" + System.currentTimeMillis();
        return new FutureUniqueIdentifier(identifier);
    } 

    public FutureUniqueIdentifier(String id)
    {
        this.identifier = id;
    }

    public String toString()
    {
        return this.identifier;
    }

    public boolean equals(FutureUniqueIdentifier other)
    {
        return this.identifier.equals(other.toString());
    }
}
