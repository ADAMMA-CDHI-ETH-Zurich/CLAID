#pragma once

#include "Utilities/ITCChannel.hpp"
#include "Runnable.hpp"
#include "ScheduledRunnable.hpp"
#include "RunnableDispatcher.hpp"

#include <thread>
#include <memory.h>
#include <functional>

namespace claid
{
    class RunnableDispatcherThread
    {
        private:
            std::thread thread;
            RunnableDispatcher runnableDispatcher;

            bool active = false;
            
            

            void run();


        public:
            void start();
            void stop();
            void join();
            void addRunnable(ScheduledRunnable scheduledRunnable);
            void addRunnable(std::shared_ptr<Runnable> runnable, std::shared_ptr<ScheduleDescription> schedule);


            bool isRunning() const;

            std::thread::id getThreadID();
    };
}