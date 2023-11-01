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

#include <iostream>
#include <map>
#include <functional>
#include <chrono>
#include "dispatch/core/Module/RunnableDispatcherThread.hpp"
#include "dispatch/core/Module/RunnableDispatcher.hpp"
#include "Logger/Logger.hpp"


namespace claid 
{
    class Module 
    {
    protected:
        std::string id = "unknown";

        RunnableDispatcher runnableDispatcher;
        bool isInitializing = false;
        bool isInitialized = false;

        std::map<std::string, ScheduledRunnable> timers;

        ChannelSubscriberPublisher* subscriberPublisher;

        void enqueueRunnable(const ScheduledRunnable& runnable);


    public:
        Module() {}

        void moduleError(const std::string& error);
        void moduleWarning(const std::string& warning);

        void setId(const std::string& id);
        std::string getId();

        bool start(ChannelSubscriberPublisher* subscriberPublisher, const std::map<std::string, std::string>& properties);



        template<typename T>
        Channel<T> publish(const std::string& channelName) 
        {
            if (!isInitializing) {
                moduleError("Cannot publish channel \"" + channelName + "\". Publishing is only allowed during initialization (i.e., the first call of the initialize function).");
                return Channel<T>::newInvalidChannel(channelName);
            }
            return subscriberPublisher.publish(this, channelName);
        }

        template<typename T>
        Channel<T> subscribe(const std::string& channelName, std::function<void(ChannelData<T>)> callback) {
            if (!isInitializing) {
                moduleError("Cannot subscribe channel \"" + channelName + "\". Subscribing is only allowed during initialization (i.e., the first call of the initialize function).");
                return Channel<T>::newInvalidChannel(channelName);
            }
            return subscriberPublisher.subscribe(this, channelName, callback, runnableDispatcher);
        }

        void registerPeriodicFunction(const std::string& name, std::function<void()> callback, const Duration& interval) {
            registerPeriodicFunction(name, callback, interval, std::chrono::system_clock::now() + interval);
        }

        void registerPeriodicFunction(const std::string& name, std::function<void()> function, const Duration& interval, const Time& startTime) {
            if (interval.count() == 0) {
                moduleError("Error in registerPeriodicFunction: Cannot register periodic function \"" + name + "\" with a period of 0 milliseconds.");
            }

            auto it = timers.find(name);
            if (it != timers.end()) {
                moduleError("Tried to register function with name \"" + name + "\", but a periodic function with the same name was already registered before.");
            }

            std::shared_ptr<FunctionRunnable<void>> functionRunnable(new FunctionRunnable<void>(function));
            std::shared_ptr<Runnable> runnable = std::static_pointer_cast<Runnable>(functionRunnable);

            

            this->timers.insert(std::make_pair(name, runnable));
            this->runnableDispatcherThread->addRunnable(ScheduledRunnable(runnable, 
                ScheduleRepeatedIntervall(firstExecutionTime, Duration(std::chrono::milliseconds(periodInMs)))));
        }

        template<typename Class>
        void registerPeriodicFunction(const std::string& name, void (Class::* f)(), Class* obj, const Duration& interval)
        {
            registerPeriodicFunction(name, f, obj, std::chrono::system_clock::now() + interval);
        }

        template<typename Class>
        void registerPeriodicFunction(const std::string& name, void (Class::* f)(), Class* obj, const Duration& interval, const Time& startTime)
        {
            std::function<void()> function = std::bind(f, obj);
            this->registerPeriodicFunction(name, function, periodInMs, startTime);
        }

        void unregisterPeriodicFunction(const std::string& name) {
            auto it = timers.find(name);
            if (it == timers.end()) {
                moduleError("Error, tried to unregister periodic function but function was not found in the list of registered timers. Was a function with this name ever registered before?");
            }

            it->second.invalidate();
            timers.erase(it);
        }

        void registerScheduledFunction(const std::string& name, const LocalDateTime& dateTime, std::function<void()> function) {
            if (dateTime < LocalDateTime::now()) {
                moduleWarning("Failed to schedule function \"" + name + "\" at time " + dateTime + ". The time is in the past. It is now: " + LocalDateTime::now());
            }

            FunctionRunnable runnable(function, std::make_shared<ScheduleOnce>(dateTime));
            timers[name] = runnable;
        }
    };

}
