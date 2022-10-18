#include "RunnableDispatcherThread/DispatcherThreadTimer.hpp"

namespace claid
{
    DispatcherThreadTimer::DispatcherThreadTimer()
    {
        
    }

    DispatcherThreadTimer::DispatcherThreadTimer(std::shared_ptr<RunnableDispatcherThread> runnableDispatcherThread, Runnable* runnable, size_t periodInMs) : runnableDispatcherThread(runnableDispatcherThread), runnable(runnable), TimerBase(periodInMs)
    {

    }

    void DispatcherThreadTimer::run()
    {
        this->runnableDispatcherThread->addRunnable(runnable);
    }
}