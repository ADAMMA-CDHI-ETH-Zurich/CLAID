// #include "RunnableDispatcherThread/DispatcherThreadTimer.hpp"

// namespace claid
// {
//     DispatcherThreadTimer::DispatcherThreadTimer()
//     {
        
//     }

//     DispatcherThreadTimer::DispatcherThreadTimer(std::shared_ptr<RunnableDispatcherThread> runnableDispatcherThread, 
//     Runnable* runnable, size_t periodInMs, bool sequentialExecution) : runnableDispatcherThread(runnableDispatcherThread), runnable(runnable), TimerBase(periodInMs), sequentialExecution(sequentialExecution)
//     {

//     }

//     void DispatcherThreadTimer::run()
//     {
//         if(this->sequentialExecution && !this->isFirstExecution)
//         {
//             if(!runnable->wasExecuted)
//             {
//                 return;
//             }
//         }
//         runnable->wasExecuted = false;
//         this->isFirstExecution = false;
//         this->runnableDispatcherThread->addRunnable(runnable);
//     }
// }