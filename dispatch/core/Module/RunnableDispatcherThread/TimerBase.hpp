/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions
* 
* Authors: Patrick Langer
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*         http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
***************************************************************************/
#pragma once

#include <chrono>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <mutex>

namespace claid
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

