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

    void TimerBase::runInternal()
    {
        while(this->active)
        {
            this->run();
            std::this_thread::sleep_for(std::chrono::milliseconds(this->periodInMs));
        }
    }
}