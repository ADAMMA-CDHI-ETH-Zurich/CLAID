package adamma.c4dhi.claid.Module;

import adamma.c4dhi.claid.Logger.Logger;

public class RunnableDispatcher 
{
    ThreadSafeChannel<Runnable> threadSafeChannel = new ThreadSafeChannel<>();
    Thread thread;
    boolean running = false;

    public void addRunnable(Runnable runnable)
    {
        threadSafeChannel.add(runnable);
    }

    public boolean start()
    {   
        if(running)
        {
            return false;
        }

        running = true;
        thread = new Thread(){
            public void run(){
              processRunnables();
            }
          };
        
        thread.start();
        return true;
    }

    public boolean stop()
    {
        if(!running)
        {
            return false;
        }

        running = false;
        try {
            thread.join();
        } catch (InterruptedException e) {
            Logger.logError("RunnableDispatcher: Failed to stop thread.");
            return false;
        }

        return true;
    }

    private void processRunnables()
    {
        while(running)
        {
            System.out.println("Runnable dispatcher processing runnables");
            Runnable runnable = threadSafeChannel.blockingGet();
            runnable.run();
        }
    }
}
