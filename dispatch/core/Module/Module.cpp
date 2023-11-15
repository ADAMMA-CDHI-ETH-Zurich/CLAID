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

#include "dispatch/core/Module/Module.hpp"

namespace claid
{
    // Somehow compiler does not like the constructor in the Module_imp .. soooo
    Module::Module()
    {
    }


    void Module::moduleError(const std::string& error) 
    {
        std::string errorMsg = "Module \"" + id + "\": " + error;
        Logger::log(SeverityLevel::ERROR, errorMsg);
    }

    void Module::moduleWarning(const std::string& warning) 
    {
        std::string warningMsg = "Module \"" + id + "\": " + warning;
        Logger::log(SeverityLevel::WARNING, warningMsg);
    }

    bool Module::start(ChannelSubscriberPublisher* subscriberPublisher, const std::map<std::string, std::string>& properties) 
    {
        if (this->isInitialized) 
        {
            moduleError("Initialize called twice!");
            return false;
        }

        this->subscriberPublisher = subscriberPublisher;

        if (!runnableDispatcher.start()) 
        {
            moduleError("Failed to start RunnableDispatcher.");
            return false;
        }

        this->isInitializing = true;
        this->isInitialized = false;

        this->runnableDispatcher.addRunnable(FunctionRunnable([this, properties] { initializeInternal(properties); }));

        while (!isInitialized) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }

        this->isInitializing = false;
        this->subscriberPublisher = nullptr;
        return true;
    }

    void Module::initializeInternal(const std::map<std::string, std::string>& properties)
    {
        this.initialize(properties);
        this.isInitialized = true;
    }

    void Module::initialize(const std::map<std::string, std::string>& properties)
    {
        System.out.println("Module base initialize");
    }

    void Module::setId(const std::string& id) {
        this->id = id;
    }

    std::string Module::getId() {
        return id;
    }

    void Module::enqueueRunnable(const ScheduledRunnable& runnable) 
    {
        runnableDispatcher.addRunnable(runnable);
    }

    void Module::registerPeriodicFunction(const std::string& name, std::function<void()> callback, const Duration& interval) 
    {
        registerPeriodicFunction(name, callback, interval, std::chrono::system_clock::now() + interval);
    }

    void Module::registerPeriodicFunction(const std::string& name, std::function<void()> function, const Duration& interval, const Time& startTime) 
    {
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

    void Module::registerScheduledFunction(const std::string& name, const LocalDateTime& dateTime, std::function<void()> function) 
    {
        if (dateTime < LocalDateTime::now()) {
            moduleWarning("Failed to schedule function \"" + name + "\" at time " + dateTime + ". The time is in the past. It is now: " + LocalDateTime::now());
        }

        FunctionRunnable runnable(function, std::make_shared<ScheduleOnce>(dateTime));
        timers[name] = runnable;
    }

    void Module::unregisterPeriodicFunction(const std::string& name) 
    {
        auto it = timers.find(name);
        if (it == timers.end()) {
            moduleError("Error, tried to unregister periodic function but function was not found in the list of registered timers. Was a function with this name ever registered before?");
        }

        it->second.invalidate();
        timers.erase(it);
    }
}

