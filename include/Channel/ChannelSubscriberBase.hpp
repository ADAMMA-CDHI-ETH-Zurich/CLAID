#include "RunnableDispatcherThread/RunnableDispatcherThread.hpp"

namespace portaible
{
    class ChannelSubscriberBase
    {   public:
            RunnableDispatcherThread* runnableDispatcherThread;

        public:
            void signalNewDataIsAvailable()
            {
                runnableDispatcherThread->addRunnable(this->asRunnable());
            }

            virtual Runnable* asRunnable() = 0;

            ChannelSubscriberBase()
            {

            }

            ChannelSubscriberBase(RunnableDispatcherThread* runnableDispatcherThread) : runnableDispatcherThread(runnableDispatcherThread)
            {
                
            }
    };
}