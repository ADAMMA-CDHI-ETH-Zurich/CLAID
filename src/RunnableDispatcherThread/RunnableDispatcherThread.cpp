#include "RunnableDispatcherThread/RunnableDispatcherThread.hpp"


namespace portaible
{
    void RunnableDispatcherThread::start()
    {
        this->thread = std::thread(&RunnableDispatcherThread::run, this);
    }

    void RunnableDispatcherThread::run()
    {
        while(true)
        {

            Runnable* runnable;

            runnablesChannel.get(runnable);

            if(runnable->isValid())
                runnable->run();

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
}