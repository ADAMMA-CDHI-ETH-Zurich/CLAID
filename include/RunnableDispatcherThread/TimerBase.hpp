#pragma once

#include <chrono>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <mutex>

namespace portaible
{
    // Class that calls the run function after a period defined by periodInMs.
    // Can be interrupted any time, even within the period (i.e., while the timer thread is waiting).
    class TimerBase
    {
        private:
            size_t periodInMs;
            std::thread thread;
            std::mutex mutex;
            std::condition_variable cv;


            bool active = false;

            void runInternal();

        public:
            TimerBase();
            TimerBase(size_t periodInMs);

            void start();
            void stop();
            virtual void run() = 0;
    };
}

