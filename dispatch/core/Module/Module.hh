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
#include "dispatch/core/Module/RunnableDispatcherThread/RunnableDispatcher.hh"
#include "dispatch/core/Logger/Logger.hh"
#include "dispatch/core/Module/ModuleRef.hh"


namespace claid
{
    class Module;
}

#include "dispatch/core/Module/ChannelSubscriberPublisher.hh"


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
        Module();

        void moduleError(const std::string& error) const;
        void moduleWarning(const std::string& warning) const;

        void setId(const std::string& id);
        std::string getId() const;

        bool start(ChannelSubscriberPublisher* subscriberPublisher, const std::map<std::string, std::string>& properties);
        void shutdown();

    protected:


        void initializeInternal(const std::map<std::string, std::string>& properties);
        virtual void initialize(const std::map<std::string, std::string>& properties);

        virtual void terminate();

        void registerPeriodicFunction(const std::string& name, std::function<void()> callback, const Duration& interval);
        void registerPeriodicFunction(const std::string& name, std::function<void()> function, const Duration& interval, const Time& startTime);

        void registerScheduledFunction(const std::string& name, const Time& dateTime, std::function<void()> function);
        void unregisterPeriodicFunction(const std::string& name);

        template<typename T>
        Channel<T> publish(const std::string& channelName) 
        {
            if (!isInitializing) {
                moduleError("Cannot publish channel \"" + channelName + "\". Publishing is only allowed during initialization (i.e., the first call of the initialize function).");
                return Channel<T>::newInvalidChannel(channelName);
            }
            return subscriberPublisher->publish<T>(ModuleRef(this), channelName);
        }

        template<typename T>
        Channel<T> subscribe(const std::string& channelName, std::function<void(ChannelData<T>)> callback) {
            if (!isInitializing) {
                moduleError("Cannot subscribe channel \"" + channelName + "\". Subscribing is only allowed during initialization (i.e., the first call of the initialize function).");
                return Channel<T>::newInvalidChannel(channelName);
            }
            return subscriberPublisher->subscribe<T>(ModuleRef(this), channelName, callback, runnableDispatcher);
        }

        template<typename Class>
        void registerPeriodicFunction(const std::string& name, void (Class::* f)(), Class* obj, const Duration& interval)
        {
            registerPeriodicFunction(name, f, obj, interval, Time::now() + interval);
        }

        template<typename Class>
        void registerPeriodicFunction(const std::string& name, void (Class::* f)(), Class* obj, const Duration& interval, const Time& startTime)
        {
            std::function<void()> function = std::bind(f, obj);
            this->registerPeriodicFunction(name, function, interval, startTime);
        }

        
    };

}
#include "dispatch/core/Module/ModuleFactory/ModuleFactory.hh"

#define REGISTER_MODULE(moduleName, className)\
    static_assert(std::is_base_of<claid::Module, className>::value, "Tried to register a class as Module (see above), which did not inherit from BaseModule. Did you forget to inherit from Module or BaseModule?");\
    REGISTER_MODULE_FACTORY(moduleName, className)

#define REGISTER_MODULE_CUSTOM_NAME(className)\
    static_assert(std::is_base_of<claid::Module, className>::value, "Tried to register a class as Module (see above), which did not inherit from BaseModule. Did you forget to inherit from Module or BaseModule?");\
    REGISTER_MODULE_FACTORY_CUSTOM_NAME(className)
