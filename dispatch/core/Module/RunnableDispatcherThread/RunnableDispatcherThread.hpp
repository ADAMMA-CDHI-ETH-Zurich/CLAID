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