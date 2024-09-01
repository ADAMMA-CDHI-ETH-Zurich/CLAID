/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Stephan Altmüller
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

#include <mutex>
#include <chrono>
#include <map>
#include <vector>
#include <iostream>
#include <thread>
#include <condition_variable>

#include "dispatch/core/Utilities/Time.hh"
#include "dispatch/core/Logger/Logger.hh"
#include "dispatch/core/Module/RunnableDispatcherThread/ScheduledRunnable.hh"
#include "dispatch/core/Module/TypeMapping/Mutator.hh"
#include "dispatch/core/Module/TypeMapping/TypeMapping.hh"

#include "dispatch/core/RemoteFunction/RemoteFunction.hh"
#include "dispatch/core/RemoteFunction/RemoteFunctionHandler.hh"

using namespace claidservice;

namespace claid
{
    class RunnableDispatcher
    {
        private:
            // size_t will be the execution time (unix timestamp in milliseconds)
            // map is sorted in increasing order.
            // Note: This has to be a multimap, because multiple runnables can have the same execution time.
            std::multimap<Time, ScheduledRunnable> scheduledRunnables;

            std::mutex mutex;
            std::condition_variable conditionVariable;

            bool rescheduleRequired = false;
            bool stopped = false;

            std::thread thread;

            // Allows the RunnableDispatcher to make calls into the Middleware, e.g.,
            // to schedule device wake ups or request to keep the device awake.
            RemoteFunctionHandler* remoteFunctionHandler;

            RemoteFunction<void> middlewareFuncAcquireWakeLock;
            RemoteFunction<void> middlewareFuncReleaseWakeLock;
            RemoteFunction<void> middlewareFuncScheduleDeviceWakeUpAt;


            std::chrono::microseconds getWaitDurationUntilNextRunnableIsDue()
            {
                std::unique_lock<std::mutex> lock(this->mutex);
                if(this->scheduledRunnables.size() == 0)
                {
                    // Wait forever.
                    return std::chrono::microseconds::max();
                }

                std::chrono::microseconds
                    microsecondsUntilNextRunnableIsDue =
                        std::chrono::duration_cast<std::chrono::microseconds>(
                            this->scheduledRunnables.begin()->first - Time::now());


                // Make sure we do not return a negative duration.
                if(microsecondsUntilNextRunnableIsDue.count() < 0)
                {
                    return std::chrono::microseconds(0);
                }
                else
                {
                    return microsecondsUntilNextRunnableIsDue;
                }
            }

            Time getExecutionTimeOfNextDueRunnable()
            {
                std::unique_lock<std::mutex> lock(this->mutex);
                if(this->scheduledRunnables.size() == 0)
                {
                    // Wait forever.
                    return Time::eternity();
                }

                return this->scheduledRunnables.begin()->first;
            }

            void waitUntilRunnableIsDueOrRescheduleIsRequired()
            {
                // If there is no runnable added, this will be infinity.
                // Hence, we will wait forever and wake up if a reschedule is required.
                Time executionTime = getExecutionTimeOfNextDueRunnable();

                // std::cout << "wait time: %u ms" << waitTime.count() / 1000 << "\n";
                // The return value of wait_for can be used to determine whether the wait exited because time passed (false),
                // or because the predicate (this->scheduleRequired) evaluates to true (true).
                // However, we are not interested in distinguishing the two cases.
                // In any case, when we wake up, we see if we need to execute anything.
                // wait_for will atomically release the mutex and sleep, and will atomically lock the mutex after waiting.
                std::unique_lock<std::mutex> lock(this->mutex);
                // Middleware: ScheduleWakeUpInMicroSeconds();
              

                middlewareScheduleDeviceWakeupAt(executionTime.toUnixTimestampMilliseconds());
                Logger::logInfo("%lu sleeping", this);
                this->conditionVariable.wait_until(lock, executionTime, [&]{return this->rescheduleRequired || this->stopped;});    
                                Logger::logInfo("%lu woke up", this);

            }

            void processRunnable(ScheduledRunnable& scheduledRunnable)
            {
                if(this->stopped)
                {
                    return;
                }

                if(scheduledRunnable.runnable->isValid())
                {
                    if(scheduledRunnable.runnable->catchExceptions)
                    {
                        try
                        {
                            Logger::logInfo("Running runnable! Remaining runnables: %d", scheduledRunnables.size());
                            scheduledRunnable.runnable->run();
                        }
                        catch(std::exception& e)
                        {
                            scheduledRunnable.runnable->setException(e.what());
                        }
                    }
                    else
                    {
                        scheduledRunnable.runnable->run();
                    }

                    scheduledRunnable.runnable->wasExecuted = true;

                    if(scheduledRunnable.runnable->stopDispatcherAfterThisRunnable)
                    {
                        // No more runnables will be executed after this one !
                        this->stop();
                        printf("STOPPED DISPATCHER!");
                        return;
                    }

                    if(scheduledRunnable.schedule->doesRunnableHaveToBeRepeated())
                    {
                        std::unique_lock<std::mutex> lock(this->mutex);

                        Time plannedExecutionTime = scheduledRunnable.schedule->getExecutionTime();
                        scheduledRunnable.schedule->updateExecutionTime();

                        std::string plannedExecutionTimeStr = plannedExecutionTime.strftime("%d.%m.%y %H:%M:%S");
                        std::string currentExecutionStr = Time::now().strftime("%d.%m.%y %H:%M:%S");

                        Time newExecutionTime = scheduledRunnable.schedule->getExecutionTime();
                        std::string nextExecutionStr = newExecutionTime.strftime("%d.%m.%y %H:%M:%S");


                        Logger::logInfo("Runnable, scheduled for execution at %s, has been executed at %s, scheduling next execution for %s", plannedExecutionTimeStr.c_str(), currentExecutionStr.c_str(), nextExecutionStr.c_str());

                        // Reinsert the runnable with new scheduled execution time.
                        // Note, that the runnable was removed from scheduledRunnables in the getAndRemoveDueRunnables() function.
                        this->scheduledRunnables.insert(std::make_pair(newExecutionTime, scheduledRunnable));
                    }
                }
            }

            void processRunnables(std::vector<ScheduledRunnable>& runnables)
            {
                int ctr = 0;
                for(ScheduledRunnable& runnable : runnables)
                {
                    Logger::logInfo("Processing runnable %d", ctr);
                    ctr++;
                    processRunnable(runnable);
                }
            }

            void getAndRemoveDueRunnables(std::vector<ScheduledRunnable>& runnables)
            {
                runnables.clear();
                Time now = Time::now();

                std::unique_lock<std::mutex> lock(this->mutex);

                auto iterator = this->scheduledRunnables.begin();

                while(iterator != this->scheduledRunnables.end())
                {
                    const Time& dueTime = iterator->first;

                    if(now >= dueTime)
                    {
                        runnables.push_back(iterator->second);
                    }
                    else
                    {
                        // Since the map is ordered in increasing order,
                        // all subsequent entries have a time
                        // which is greater than the time of the current entry.
                        // Hence, no need to keep iterating. We found the latest-most runnable
                        // which is due.
                        break;
                    }

                    iterator++;
                }

                if(runnables.size() > 0)
                {
                    Logger::logInfo("Found %d due runnables out of %d runnables in total: ", runnables.size(), scheduledRunnables.size());
                    // If at least one runnable was added to runnables, that means we have to remove them from scheduledRunnables.
                    this->scheduledRunnables.erase(this->scheduledRunnables.begin(), iterator);
                    Logger::logInfo("Removed %d due runnables, number of remaining runnables is %d ", runnables.size(), scheduledRunnables.size());
                }

                // // upper_bound returns an iterator to the first element greater than now
                // auto it = this->scheduledRunnables.upper_bound(now);

            }

            void runScheduling()
            {
                Logger::logInfo("Running scheduling");
                std::vector<ScheduledRunnable> dueRunnables;
                while(!stopped)
                {
                    Logger::logInfo("Blablabla");
                    middlewareAcquireWakeLock();
                    do
                    {
                        // While we process runnables, it is possible
                        // that another runnable becomes due in the meantime.
                        // Hence, we repeat this loop until there are no more
                        // runnables that are due.
                        getAndRemoveDueRunnables(dueRunnables);

                        processRunnables(dueRunnables);
                    }
                    while(dueRunnables.size() != 0);

                    rescheduleRequired = false;
                    middlewareReleaseWakeLock();
                    waitUntilRunnableIsDueOrRescheduleIsRequired();
                }
                Logger::logInfo("RunnableDispatcher shutdown.");
            }

            RuntimeType getRuntimeType()
            {
                RuntimeType type;
                type.set_runtime(RUNTIME_CPP);
                return type;
            }

            void middlewareAcquireWakeLock()
            {
                if(this->remoteFunctionHandler == nullptr)
                {
                    return;
                }
                middlewareFuncAcquireWakeLock.execute(getRuntimeType());
            }

            void middlewareReleaseWakeLock()
            {
                if(this->remoteFunctionHandler == nullptr)
                {
                    return;
                }
                middlewareFuncReleaseWakeLock.execute(getRuntimeType());
            }

            void middlewareScheduleDeviceWakeupAt(int64_t milliseconds)
            {
                if(this->remoteFunctionHandler == nullptr)
                {
                    return;
                }
                middlewareFuncScheduleDeviceWakeUpAt.execute(getRuntimeType(), milliseconds);
            }

        public:

            RunnableDispatcher() : stopped(true)
            {

            }

            void setRemoteFunctionHandler(RemoteFunctionHandler* remoteFunctionHandler)
            {
                this->middlewareFuncAcquireWakeLock = 
                    remoteFunctionHandler->mapRuntimeFunction<void, RuntimeType>(Runtime::MIDDLEWARE_CORE, "acquire_wakelock");

                this->middlewareFuncReleaseWakeLock = 
                    remoteFunctionHandler->mapRuntimeFunction<void, RuntimeType>(Runtime::MIDDLEWARE_CORE, "release_wakelock");

                this->middlewareFuncScheduleDeviceWakeUpAt = 
                    remoteFunctionHandler->mapRuntimeFunction<void, RuntimeType, int64_t>(Runtime::MIDDLEWARE_CORE, "schedule_device_wakeup_at");
            }

            bool start()
            {
                if(!this->stopped)
                {
                    return false;
                }

                this->stopped = false;
                this->thread = std::thread(&RunnableDispatcher::runScheduling, this);

                return true;
            }

            bool stop()
            {
                if(this->stopped)
                {
                    return false;
                }

                this->stopped = true;
                this->conditionVariable.notify_all();
                this->thread.join();
                return true;
            }

            void addRunnable(ScheduledRunnable runnable)
            {   
                std::unique_lock<std::mutex> lock(this->mutex);
                this->scheduledRunnables.insert(std::make_pair(runnable.schedule->getExecutionTime(), runnable));
                Logger::logInfo("Added runnable to RunnableDispatcher, total runnables now: %d", scheduledRunnables.size());
                // This will lead to a wake up, so we can reschedule.
                this->rescheduleRequired = true;
                this->conditionVariable.notify_all();
            }

            void removeRunnable(ScheduledRunnable runnable)
            {
                std::unique_lock<std::mutex> lock(this->mutex);

                Time executionTime = runnable.schedule->getExecutionTime();
                auto it = this->scheduledRunnables.find(executionTime);

                if(it != this->scheduledRunnables.end())
                {
                    ScheduledRunnable& storedRunnable = it->second;
                    if(storedRunnable == runnable)
                    {
                        scheduledRunnables.erase(it);
                        this->rescheduleRequired = true;
                        // This will lead to a wake-up, so we can reschedule.
                        this->conditionVariable.notify_all();
                    }
                }
            }


    };
}