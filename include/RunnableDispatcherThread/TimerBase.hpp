#pragma once

#include <thread>

namespace portaible
{
    class TimerBase
    {
        private:
            size_t periodInMs;
            std::thread thread;

            bool active = false;

            void runInternal();

        public:
            TimerBase();
            TimerBase(size_t periodInMs);

            void start();
            virtual void run() = 0;
    };
}