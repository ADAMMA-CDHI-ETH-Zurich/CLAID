#include "RunnableDispatcherThread/RunnableDispatcherThread.hpp"

namespace portaible
{
    class Subscriber
    {   public:
            std::shared_ptr<RunnableDispatcherThread> runnableDispatcherThread;

        public:
            virtual void signalNewDataIsAvailable()
            {
                runnableDispatcherThread->addRunnable(this->asRunnable());
            }

            virtual Runnable* asRunnable() = 0;

            Subscriber()
            {

            }

            Subscriber(std::shared_ptr<RunnableDispatcherThread> runnableDispatcherThread) : runnableDispatcherThread(runnableDispatcherThread)
            {
                
            }
    };
}