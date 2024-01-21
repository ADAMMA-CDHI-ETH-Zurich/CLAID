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

#include "dispatch/core/Module/Module.hh"
#include "dispatch/core/Module/RunnableDispatcherThread/FunctionRunnable.hh"
#include "dispatch/core/Module/RunnableDispatcherThread/ScheduleDescription/ScheduleOnce.hh"
#include "dispatch/core/Module/RunnableDispatcherThread/ScheduleDescription/ScheduleRepeatedIntervall.hh"
#include "dispatch/core/Exception/Exception.hh"
#include "absl/strings/str_split.h"

namespace claid
{
    Module::Module()
    {
    }

    void Module::moduleFatal(const std::string& error) const
    {
        std::string errorMsg = "Module \"" + id + "\": " + error;
        Logger::log(LogMessageSeverityLevel::FATAL, errorMsg, LogMessageEntityType::MODULE, this->id);
        CLAID_THROW(claid::Exception, errorMsg);
    }

    void Module::moduleError(const std::string& error) const
    {
        std::string errorMsg = "Module \"" + id + "\": " + error;
        Logger::log(LogMessageSeverityLevel::ERROR, errorMsg, LogMessageEntityType::MODULE, this->id);
    }

    void Module::moduleWarning(const std::string& warning) const
    {
        std::string warningMsg = "Module \"" + id + "\": " + warning;
        Logger::log(LogMessageSeverityLevel::WARNING, warningMsg, LogMessageEntityType::MODULE, this->id);
    }

    void Module::moduleInfo(const std::string& warning) const
    {
        std::string warningMsg = "Module \"" + id + "\": " + warning;
        Logger::log(LogMessageSeverityLevel::INFO, warningMsg, LogMessageEntityType::MODULE, this->id);
    }

    void Module::moduleDebug(const std::string& warning) const
    {
        std::string warningMsg = "Module \"" + id + "\": " + warning;
        Logger::log(LogMessageSeverityLevel::DEBUG_LOG, warningMsg, LogMessageEntityType::MODULE, this->id);
    }

    void Module::moduleFatal(absl::Status error) const
    {
        std::stringstream ss;
        ss << error;
        std::string errorStr = ss.str();
        this->moduleFatal(errorStr);
    }

    void Module::moduleError(absl::Status error) const
    {
        std::stringstream ss;
        ss << error;
        std::string errorStr = ss.str();
        this->moduleError(errorStr);
    }

    void Module::moduleWarning(absl::Status warning) const
    {
        std::stringstream ss;
        ss << warning;
        std::string warningStr = ss.str();
        this->moduleWarning(warningStr);
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

        std::shared_ptr<FunctionRunnable<void>> functionRunnable (new FunctionRunnable<void>([this, properties] { initializeInternal(properties); }));

        this->runnableDispatcher.addRunnable(
            ScheduledRunnable(
                std::static_pointer_cast<Runnable>(functionRunnable), 
                ScheduleOnce(Time::now())));

        while (!isInitialized) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }

        this->isInitializing = false;
        this->subscriberPublisher = nullptr;
        return true;
    }

    void Module::initializeInternal(const std::map<std::string, std::string>& properties)
    {
        this->initialize(properties);
        this->isInitialized = true;
    }

    void Module::initialize(const std::map<std::string, std::string>& properties)
    {
    }

    void Module::setId(const std::string& id) {
        this->id = id;
    }

    std::string Module::getId() const
    {
        return id;
    }

    void Module::enqueueRunnable(const ScheduledRunnable& runnable) 
    {
        runnableDispatcher.addRunnable(runnable);
    }

    void Module::registerPeriodicFunction(const std::string& name, std::function<void()> callback, const Duration& interval) 
    {
        registerPeriodicFunction(name, callback, interval, Time::now() + interval);
    }

    void Module::registerPeriodicFunction(const std::string& name, std::function<void()> function, const Duration& interval, const Time& startTime) 
    {
        if (interval.getMicroSeconds() == 0) {
            moduleError("Error in registerPeriodicFunction: Cannot register periodic function \"" + name + "\" with a period of 0 milliseconds.");
        }

        auto it = timers.find(name);
        if (it != timers.end()) {
            moduleError("Tried to register function with name \"" + name + "\", but a periodic function with the same name was already registered before.");
        }

        std::shared_ptr<FunctionRunnable<void>> functionRunnable(new FunctionRunnable<void>(function));
        std::shared_ptr<Runnable> runnable = std::static_pointer_cast<Runnable>(functionRunnable);

        ScheduledRunnable scheduledRunnable(runnable, 
            ScheduleRepeatedIntervall(startTime, interval));

        this->timers.insert(std::make_pair(name, scheduledRunnable));
        this->runnableDispatcher.addRunnable(scheduledRunnable);
        Logger::logInfo("Registered periodic runnable %s", name.c_str());
    }

    void Module::registerScheduledFunction(const std::string& name, const Time& startTime, std::function<void()> function) 
    {
        if (startTime < Time::now()) {
            moduleWarning(absl::StrCat(
                "Failed to schedule function \"", name, "\" at time ", startTime.strftime("%d.%m.%y - %H:%M:%S"), ". ", 
                "The time is in the past. It is now: ", Time::now().strftime("%d.%m.%y - %H:%M:%S")
            ));
        }

        std::shared_ptr<FunctionRunnable<void>> functionRunnable (new FunctionRunnable<void>(function));

        ScheduledRunnable scheduledRunnable(
            std::static_pointer_cast<Runnable>(functionRunnable), 
            ScheduleOnce(startTime));

        this->timers.insert(std::make_pair(name, scheduledRunnable));
        this->runnableDispatcher.addRunnable(scheduledRunnable);
    }

    void Module::unregisterPeriodicFunction(const std::string& name) 
    {
        auto it = timers.find(name);
        if (it == timers.end()) {
            moduleError("Error, tried to unregister periodic function but function was not found in the list of registered timers. Was a function with this name ever registered before?");
        }

        it->second.runnable->invalidate();
        timers.erase(it);
    }

    void Module::unregisterAllPeriodicFunctions()
    {
        for(auto& entry : this->timers)
        {
            entry.second.runnable->invalidate();
        }
    }

    void Module::shutdown()
    {
        this->isTerminating = true;

        std::shared_ptr<FunctionRunnable<void>> functionRunnable (new FunctionRunnable<void>([this] { this->terminateInternal(); }));

        this->runnableDispatcher.addRunnable(
            ScheduledRunnable(
                std::static_pointer_cast<Runnable>(functionRunnable), 
                ScheduleOnce(Time::now())));

        while(this->isTerminating)
        {
            Logger::logInfo("Terminate internal 1");
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            Logger::logInfo("Terminate internal 2");
        }
        Logger::logInfo("Runnable dispatcher stop 1");
        this->runnableDispatcher.stop();
        Logger::logInfo("Runnable dispatcher stop 2");
        this->isInitialized = false;
        Logger::logInfo("Runnable dispatcher stop 3");  

    }

    void Module::terminateInternal()
    {
        this->unregisterAllPeriodicFunctions();
        this->terminate();
        this->isTerminating = false;

    }

    void Module::terminate()
    {

    }

    void Module::onConnectedToRemoteServer()
    {

    }
  
    void Module::onDisconnectedFromRemoteServer()
    {

    }

    void Module::notifyConnectedToRemoteServer()
    {
        std::shared_ptr<FunctionRunnable<void>> functionRunnable (new FunctionRunnable<void>([this] { this->onConnectedToRemoteServer();}));

        this->runnableDispatcher.addRunnable(
            ScheduledRunnable(
                std::static_pointer_cast<Runnable>(functionRunnable), 
                ScheduleOnce(Time::now())));
    }
        
    void Module::notifyDisconnectedFromRemoteServer()
    {
        std::shared_ptr<FunctionRunnable<void>> functionRunnable (new FunctionRunnable<void>([this] { this->onDisconnectedFromRemoteServer();}));

        this->runnableDispatcher.addRunnable(
            ScheduledRunnable(
                std::static_pointer_cast<Runnable>(functionRunnable), 
                ScheduleOnce(Time::now())));
    }
}

