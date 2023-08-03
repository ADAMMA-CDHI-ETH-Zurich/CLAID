#include "TimerBase.hpp"


#include "RunnableDispatcherThread.hpp"

namespace claid
{
    class DispatcherThreadTimer : public TimerBase
    {
        private: 
            std::shared_ptr<RunnableDispatcherThread> runnableDispatcherThread;
            Runnable* runnable;

            bool sequentialExecution = false;
            bool isFirstExecution = true;
       
       
        public:
            DispatcherThreadTimer();

            DispatcherThreadTimer(std::shared_ptr<RunnableDispatcherThread> runnableDispatcherThread, Runnable* runnable, size_t periodInMs, bool sequentialExecution = false);

            void run();
    };  
}