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

            bool active = false;

            void run();

        public:
            void start();
            void stop();
            void join();
            void addRunnable(Runnable* runnable);

            bool isRunning() const;
    };
}