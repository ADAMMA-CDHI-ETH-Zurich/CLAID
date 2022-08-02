#include "RunnableDispatcherThread/TimerBase.hpp"

namespace portaible
{
    TimerBase::TimerBase()
    {
        
    }
    
    TimerBase::TimerBase(size_t periodInMs) : periodInMs(periodInMs)
    {

    }
    


    void TimerBase::start()
    {
        this->active = true;
        this->thread = std::thread(&TimerBase::runInternal, this);
    }

    void TimerBase::stop()
    {
        std::unique_lock<std::mutex> lock{this->mutex};
        this->active = false;
        lock.unlock();

        cv.notify_one();
        this->thread.join();
    }

    void TimerBase::runInternal()
    {

        auto endTime = std::chrono::system_clock::now();

        std::unique_lock<std::mutex> lock{this->mutex};
        while (!this->cv.wait_until(lock, endTime += std::chrono::milliseconds(periodInMs),
                                 [&]{ return !this->active; }))
        {
            this->run();
        }
    }
}