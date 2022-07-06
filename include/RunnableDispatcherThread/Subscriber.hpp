#include "RunnableDispatcherThread/RunnableDispatcherThread.hpp"

namespace portaible
{
    class Subscriber
    {   public:
            RunnableDispatcherThread* runnableDispatcherThread;

        public:
            void signalNewDataIsAvailable()
            {
                runnableDispatcherThread->addRunnable(this->asRunnable());
            }

            virtual Runnable* asRunnable() = 0;

            Subscriber()
            {

            }

            Subscriber(RunnableDispatcherThread* runnableDispatcherThread) : runnableDispatcherThread(runnableDispatcherThread)
            {
                
            }
    };
}