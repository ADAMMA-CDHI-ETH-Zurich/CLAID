#include "RunnableDispatcherThread/RunnableDispatcherThread.hpp"
#include "RunnableDispatcherThread/ScheduleDescription/ScheduleOnce.hpp"

namespace claid
{
    class Subscriber
    {   public:
            std::shared_ptr<RunnableDispatcherThread> runnableDispatcherThread;

        public:
            virtual void signalNewDataIsAvailable()
            {
                std::shared_ptr<Runnable> runnable = this->getRunnable();

                

                runnableDispatcherThread->addRunnable(ScheduledRunnable(runnable, ScheduleOnce(Time::now())));
            }

            virtual std::shared_ptr<Runnable> getRunnable() = 0;

            Subscriber()
            {

            }

            Subscriber(std::shared_ptr<RunnableDispatcherThread> runnableDispatcherThread) : runnableDispatcherThread(runnableDispatcherThread)
            {
                
            }
    };
}