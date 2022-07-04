#include "TimerBase.hpp"


#include "RunnableDispatcherThread.hpp"

namespace portaible
{
    class DispatcherThreadTimer : public TimerBase
    {
        private: 
            RunnableDispatcherThread* runnableDispatcherThread;
            Runnable* runnable;
       
       
        public:
            DispatcherThreadTimer();

            DispatcherThreadTimer(RunnableDispatcherThread* runnableDispatcherThread, Runnable* runnable, size_t periodInMs);

            void run();
    };  
}