#pragma once

#include "Utilities/ITCChannel.hpp"
#include "Runnable.hpp"
#include <thread>
#include <memory.h>
#include <functional>

namespace portaible
{
    class RunnableDispatcherThread
    {
        private:
            std::thread thread;
            ITCChannel<Runnable*> runnablesChannel;

            void run();

        public:
            void start();
            void addRunnable(Runnable* runnable);
            int id;
    };
}