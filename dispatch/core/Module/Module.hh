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

#include <iostream>
#include <map>
#include <functional>
#include <chrono>
#include "dispatch/core/Module/RunnableDispatcherThread/RunnableDispatcher.hh"
#include "dispatch/core/Logger/Logger.hh"
#include "dispatch/core/Module/ModuleRef.hh"
#include "dispatch/core/Module/Publisher.hh"
#include "dispatch/core/Module/Subscriber.hh"
#include "dispatch/core/Module/Properties.hh"
#include "dispatch/core/EventTracker/EventTracker.hh"
#include "dispatch/core/RemoteFunction/RemoteFunction.hh"
#include "dispatch/core/RemoteFunction/RemoteFunctionHandler.hh"
#include "dispatch/core/RemoteFunction/RemoteFunctionRunnableHandler.hh"
#include "dispatch/core/Utilities/ScheduleHelper.hh"

namespace claid
{
    class Module;
}

#include "dispatch/core/Module/ChannelSubscriberPublisher.hh"

using claidservice::PowerProfile;
using claidservice::Schedule;

namespace claid 
{
    class Module 
    {
    protected:
        std::string id = "unknown";
        std::string type = "unknown";

        RunnableDispatcher runnableDispatcher;
        bool isInitializing = false;
        bool isInitialized = false;
        bool isTerminating = false;

        bool isPaused = false;

        std::map<std::string, ScheduledRunnable> timers;

        ChannelSubscriberPublisher* subscriberPublisher;
        RemoteFunctionHandler* remoteFunctionHandler;
        std::shared_ptr<RemoteFunctionRunnableHandler> remoteFunctionRunnableHandler;

        std::shared_ptr<EventTracker> eventTracker;

        void enqueueRunnable(const ScheduledRunnable& runnable);

    public:
        Module();
        virtual ~Module() {}
        
        void moduleFatal(const std::string& error) const;
        void moduleError(const std::string& error) const;
        void moduleWarning(const std::string& warning) const;

        void moduleInfo(const std::string& info) const;
        void moduleDebug(const std::string& debug) const;

        void moduleFatal(absl::Status error) const;
        void moduleError(absl::Status error) const;
        void moduleWarning(absl::Status warning) const;

        void setId(const std::string& id);
        std::string getId() const;

        void setType(const std::string& type);
        std::string getType() const;

        void setEventTracker(std::shared_ptr<EventTracker> eventTracker);

        void shutdown();

        void notifyConnectedToRemoteServer();
        void notifyDisconnectedFromRemoteServer();

        void pauseModule();
        void resumeModule();
        void adjustPowerProfile(PowerProfile powerProfile);

        virtual bool start(ChannelSubscriberPublisher* subscriberPublisher, RemoteFunctionHandler* remoteFunctionHandler, Properties properties);

        void enqueueRPC(std::shared_ptr<DataPackage> rpcRequest)
        {
            std::function<void (std::shared_ptr<DataPackage>)> callback = 
                std::bind(&Module::executeRPCRequest, this, std::placeholders::_1);

            std::shared_ptr<
                    FunctionRunnableWithParams<void, std::shared_ptr<DataPackage>>> functionRunnable =
                            std::make_shared<FunctionRunnableWithParams<void, std::shared_ptr<DataPackage>>>(callback);

            functionRunnable->setParams(rpcRequest);
 

            std::shared_ptr<Runnable> runnable = std::static_pointer_cast<Runnable>(functionRunnable);
            
                
            this->runnableDispatcher.addRunnable(
                ScheduledRunnable(
                    std::static_pointer_cast<Runnable>(functionRunnable), 
                    ScheduleOnce(Time::now())));
        }

    protected:


        void initializeInternal(Properties properties);
        virtual void initialize(Properties properties) = 0;

        void terminateInternal();
        virtual void terminate();

        virtual void onConnectedToRemoteServer();
        virtual void onDisconnectedFromRemoteServer();

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

        void registerPeriodicFunction(const std::string& name, std::function<void()> callback, const Duration& interval);
        void registerPeriodicFunction(const std::string& name, std::function<void()> function, const Duration& interval, const Time& startTime);
        void registerPeriodicFunction(const std::string& name, std::function<void()> function, 
            const Duration& interval, const Time& startTime, const ScheduleTimeWindow& onlyActiveBetweenFimeFrame);

        void unregisterPeriodicFunction(const std::string& name);
        void unregisterAllPeriodicFunctions();

        void registerScheduledFunction(const std::string& name, const Time& dateTime, std::function<void()> function);

        template<typename Class>
        void registerScheduledFunction(const std::string& name, const Time& dateTime,
                        void (Class::*f)(), Class* obj)
        {
            std::function<void()> function = std::bind(f, obj);
            registerScheduledFunction(name, dateTime, function);  
        }

        /// Registers a function to be executed based on a Schedule.
        /// The Schedule can contain multiple periodic and/or timed schedules.
        /// The specified function will be called at all times as specified in the Schedule.
        /// The registerFunctionBasedOnSchedule returns a list of strings corresponding to the name under which each
        /// entry of the Schedule was registered to the local list of timers.
        /// The names in the returned list can be used to individually unsubscribe any of the registered timed or periodic ScheduledRunnables.
        /// The provided "name" parameter will be used as prefix for all names in the returned list.
        std::vector<std::string> registerFunctionBasedOnSchedule(
                const std::string& name, const Schedule& schedule, std::function<void()> function)
        {   
            std::vector<std::string> timerNames;

            // Counter and timestamp are used to provide a unique name for the registered function.
            const Time now = Time::now();
            uint64_t timestamp = now.toUnixTimestampMilliseconds();
            for(const SchedulePeriodic& schedulePeriodic : schedule.periodic())
            {
                std::string functionName = name + std::string("_") + std::to_string(timestamp) + std::string("_") + std::to_string(timers.size());
                timerNames.push_back(functionName);

                if(schedulePeriodic.interval_case() == SchedulePeriodic::IntervalCase::INTERVAL_NOT_SET)
                {
                    moduleFatal(absl::StrCat(
                        "Cannot register periodic function \"", name, "\" based on schedule; ",
                        "no interval (i.e., frequency OR period) specified for scedule."
                    ));
                    return {};
                }

                const Duration interval = ScheduleHelper::getIntervalDurationFromPeriodicSchedule(schedulePeriodic);

                Time startTime = now;
                if(schedulePeriodic.has_first_execution_time_of_day())
                {
                    startTime = ScheduleHelper::calculateNextTimeOfDay(schedulePeriodic.first_execution_time_of_day());
                }

                if(schedulePeriodic.has_only_active_between_time_frame())
                {
                    registerPeriodicFunction(functionName, function, interval, startTime,  schedulePeriodic.only_active_between_time_frame());
                }
                else
                {
                    registerPeriodicFunction(functionName, function, interval, startTime);
                }

            }

            for(const ScheduleExactTime& scheduleExactTime : schedule.timed())
            {
                std::string functionName = name + std::string("_") + std::to_string(timestamp) + std::string("_") + std::to_string(timers.size());
                timerNames.push_back(functionName);
                Time startTime = now;
                startTime = ScheduleHelper::calculateNextTimeOfDay(scheduleExactTime.time_of_day());

                if(scheduleExactTime.repeat_every_n_days() != 0)
                {
                    // If we have to repeat the function, schedule it as periodic function.
                    registerPeriodicFunction(functionName, function, Duration::days(1), startTime);
                }
                else
                {
                    // Otherwise, schedule it exactly once.
                    registerScheduledFunction(functionName, startTime, function);
                }
            }

            return timerNames;
        }

        template<typename Class>
        std::vector<std::string> registerFunctionBasedOnSchedule(
                const std::string& name, const Schedule& schedule, void (Class::*f)(), Class* obj)
        {   
            std::function<void()> function = std::bind(f, obj);
            return registerFunctionBasedOnSchedule(name, schedule, function);  
        }

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

            std::shared_ptr<Subscriber<T>> subscriber = std::make_shared<Subscriber<T>>(callback, runnableDispatcher);
            return subscriberPublisher->subscribe<T>(ModuleRef(this), channelName, subscriber);
        }

        
        template<typename T, typename Class>
        Channel<T> subscribe(const std::string& channelID,
                        void (Class::*f)(ChannelData<T>), Class* obj)
        {
            std::function<void (ChannelData<T>)> function = std::bind(f, obj, std::placeholders::_1);
            return subscribe(channelID, function); 
        }


        void pauseInternal();
        void resumeInternal();

        virtual void onPause();
        virtual void onResume();
        virtual void onPowerProfileChanged(PowerProfile powerProfile);

        std::string getCommonDataPath() const;

    // 
        void executeRPCRequest(std::shared_ptr<DataPackage> rpcRequest)
        {
            if(rpcRequest->target_module() != this->id)
            {
                moduleError("Failed to execute RPC request. RPC is targeted for Module \"" + rpcRequest->source_module() +
                    "\", but we are Module \"" + this->id + "\".");
                return;
            }

            bool result = this->remoteFunctionRunnableHandler->executeRemoteFunctionRunnable(rpcRequest);
            if(!result)
            {
                moduleError("Failed to execute rpcRequest");
                return;
            }

        }

        template<typename Class, typename Return, typename... Parameters>
        bool registerRemoteFunction(std::string functionName,  Return (Class::*f)(Parameters...), Class* obj)
        {
            return this->remoteFunctionRunnableHandler->registerRunnable(functionName, f, obj);
        }

        template<typename Return, typename... Parameters>
        RemoteFunction<Return> mapRemoteFunctionOfModule(std::string moduleId, std::string functionName)
        {
            if(moduleId == this->id)
            {
                moduleFatal(absl::StrCat(
                    "Cannot map remote function. Module tried to map function \"", functionName ,"\" of itself, which is not allowed."));
                return RemoteFunction<Return>::InvalidRemoteFunction();
            }
            return this->remoteFunctionHandler->mapModuleFunction<Return, Parameters...>(moduleId, functionName);
        }

        template<typename Return, typename... Parameters>
        RemoteFunction<Return> mapRemoteFunctionOfRuntime(Runtime runtime, std::string functionName)
        {
            return this->remoteFunctionHandler->mapRuntimeFunction<Return, Parameters...>(runtime, functionName);
        }
    };

}
#include "dispatch/core/Module/ModuleFactory/ModuleFactory.hh"

#define REGISTER_MODULE(moduleName, className)\
    static_assert(std::is_base_of<claid::Module, className>::value, "Tried to register a class as Module (see above), which did not inherit from BaseModule. Did you forget to inherit from Module or BaseModule?");\
    REGISTER_MODULE_FACTORY_CUSTOM_NAME(moduleName, className)
    