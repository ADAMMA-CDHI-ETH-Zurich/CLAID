#include "RunnableDispatcherThread/DispatcherThreadTimer.hpp"

namespace portaible
{
    DispatcherThreadTimer::DispatcherThreadTimer()
    {
        
    }

    DispatcherThreadTimer::DispatcherThreadTimer(RunnableDispatcherThread* runnableDispatcherThread, Runnable* runnable, size_t periodInMs) : runnableDispatcherThread(runnableDispatcherThread), runnable(runnable), TimerBase(periodInMs)
    {

    }

    void DispatcherThreadTimer::run()
    {
        this->runnableDispatcherThread->addRunnable(runnable);
    }
}