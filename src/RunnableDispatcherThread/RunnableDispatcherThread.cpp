#include "RunnableDispatcherThread/RunnableDispatcherThread.hpp"
#include "RunnableDispatcherThread/DummyRunnable.hpp"


namespace claid
{
    void RunnableDispatcherThread::start()
    {
        if(this->active)
        {
            return;
        }
        this->active = true;
        this->thread = std::thread(&RunnableDispatcherThread::run, this);
    }

    void RunnableDispatcherThread::stop()
    {
        if(!this->active)
        {
            return;
        }
        this->active = false;
        // Insert dummy runnable, to force the thread to get active (otherwise it's blocked on runnablesChannel.get
        Runnable* runnable = static_cast<Runnable*>(new DummyRunnable());
        runnable->deleteAfterRun = true;
        this->runnablesChannel.put(runnable);
    }

    bool RunnableDispatcherThread::isRunning() const
    {
        return this->active;
    }

    void RunnableDispatcherThread::join()
    {
        this->thread.join();
    }

    void RunnableDispatcherThread::run()
    {
        while(this->active)
        {
            Runnable* runnable;

            runnablesChannel.get(runnable);

            if(runnable->isValid())
                runnable->run();

            runnable->wasExecuted = true;
            if(runnable->deleteAfterRun)
            {
                delete runnable;
            }
        }
                
    }

    void RunnableDispatcherThread::addRunnable(Runnable* runnable)
    {
        // ITCChannel is thread safe !
        this->runnablesChannel.put(runnable);
    }

    std::thread::id RunnableDispatcherThread::getThreadID()
    {
        return this->thread.get_id();
    }

}