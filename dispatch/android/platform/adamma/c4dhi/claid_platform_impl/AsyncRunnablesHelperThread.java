package adamma.c4dhi.claid_android.Permissions;
import adamma.c4dhi.claid_android.Permissions.Permission;
import adamma.c4dhi.claid.Module.ThreadSafeChannel;

// Executes Runnables in a separate thread.
public class AsyncRunnablesHelperThread 
{
    private Thread thread;
    private ThreadSafeChannel<Runnable> runnablesChannel = new ThreadSafeChannel<>();
    private boolean started = false;


    private void processRunnables()
    {
        while(this.started)
        {
            Runnable runnable = runnablesChannel.blockingGet();
            runnable.run();
        }
    }

    public void insertRunnable(Runnable runnable)
    {
        this.runnablesChannel.add(runnable);
    }

    public boolean start()
    {
        if(this.started)
        {
            return false;
        }

        this.started = true;
        this.thread = new Thread(() -> processRunnables());
        this.thread.start();
        return true;
    }

    public boolean stop()
    {
        if(!this.started)
        {
            return false;
        }

        this.started = false;

        try{
            this.thread.join();
        }
        catch(InterruptedException e)
        {
            e.printStackTrace();
        }

        return true;
    }

    public boolean isRunning()
    {
        return this.started;
    }
}
