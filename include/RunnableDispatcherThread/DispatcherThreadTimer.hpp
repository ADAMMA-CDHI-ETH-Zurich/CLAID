#include "TimerBase.hpp"


#include "RunnableDispatcherThread.hpp"

namespace portaible
{
    class DispatcherThreadTimer : public TimerBase
    {
        private: 
            std::shared_ptr<RunnableDispatcherThread> runnableDispatcherThread;
            Runnable* runnable;
       
       
        public:
            DispatcherThreadTimer();

            DispatcherThreadTimer(std::shared_ptr<RunnableDispatcherThread> runnableDispatcherThread, Runnable* runnable, size_t periodInMs);

            void run();
    };  
}