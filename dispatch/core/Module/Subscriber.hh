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

#include "AbstractSubscriber.hh"
#include "dispatch/core/Module/RunnableDispatcherThread/RunnableDispatcher.hh"
#include "dispatch/core/Module/RunnableDispatcherThread/FunctionRunnableWithParams.hh"
#include "dispatch/core/Module/RunnableDispatcherThread/ScheduleDescription/ScheduleOnce.hh"


#include "dispatch/core/Module/ChannelData.hh"
#include "dispatch/core/Module/TypeMapping/Mutator.hh"
#include "dispatch/core/Module/TypeMapping/TypeMapping.hh"

namespace claid{
    
    template<typename T>
    class Subscriber : public AbstractSubscriber
    {
    private:
        std::function<void(ChannelData<T>)> callback;
        RunnableDispatcher& callbackDispatcher;

        Mutator<T> mutator;
            
    public:
        Subscriber(std::function<void(ChannelData<T>)> callback, 
            RunnableDispatcher& callbackDispatcher) : callback(callback), callbackDispatcher(callbackDispatcher)
        {
            this->mutator = TypeMapping::getMutator<T>();
        }

        
        void invokeCallback(ChannelData<T> data)
        {
            std::shared_ptr<
                    FunctionRunnableWithParams<void, ChannelData<T>>> functionRunnable =
                            std::make_shared<FunctionRunnableWithParams<void, ChannelData<T>>>(this->callback);

            functionRunnable->setParams(data);
 

            std::shared_ptr<Runnable> runnable = std::static_pointer_cast<Runnable>(functionRunnable);
            
                
            this->callbackDispatcher.addRunnable(
                ScheduledRunnable(
                    std::static_pointer_cast<Runnable>(functionRunnable), 
                    ScheduleOnce(Time::now())));
        }

        void onNewData(std::shared_ptr<DataPackage> package) override final
        {
            std::shared_ptr<T> data = std::make_shared<T>();

            this->mutator.getPackagePayload(*package, *data);

            // Create a new copy of the data so we can take ownership.
            ChannelData<T> channelData(data, Time::fromUnixTimestampMilliseconds(package->unix_timestamp_ms()), package->source_user_token());
            this->invokeCallback(channelData);
        }
    };
}