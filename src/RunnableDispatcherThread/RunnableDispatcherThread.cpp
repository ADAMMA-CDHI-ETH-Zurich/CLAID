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
        this->runnableDispatcher.init();

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
        this->runnableDispatcher.stop();
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
        this->runnableDispatcher.runScheduling();
    }

    void RunnableDispatcherThread::addRunnable(Runnable* runnable)
    {
        // ITCChannel is thread safe !
        this->runnablesChannel.put(runnable);
    }

    void RunnableDispatcherThread::addRunnable(ScheduledRunnable scheduledRunnable)
    {
        this->runnableDispatcher.addRunnable(scheduledRunnable);
    }

    void RunnableDispatcherThread::addRunnable(
            std::shared_ptr<Runnable> runnable, std::shared_ptr<ScheduleDescription> schedule)
    [
        this->runnableDispatcher.addRunnable(ScheduledRunnable(runnable, schedule));
    ]

    std::thread::id RunnableDispatcherThread::getThreadID()
    {
        return this->thread.get_id();
    }

}

