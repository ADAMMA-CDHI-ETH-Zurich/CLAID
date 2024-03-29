
#pragma once

#include "FunctionRunnableBase.hh"
#include <functional>

namespace claid
{
    // A Runnable that does nothing.
    // Can be used to wake up a RunnableDispatcher thread, by inserting
    // a DummyRunnable into the queue.
    class DummyRunnable : public FunctionRunnableBase
    {
        private:
            void run()
            {
             
            }
          
    };
}