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
    class Subscriber
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
                    FunctionRunnableWithParams<void>> functionRunnable =
                            std::make_shared<FunctionRunnableWithParams<void>>(this->callback);


 

            std::shared_ptr<Runnable> runnable = std::static_pointer_cast<Runnable>(functionRunnable);
            
                
            this->callbackDispatcher.addRunnable(
                ScheduledRunnable(
                    std::static_pointer_cast<Runnable>(functionRunnable), 
                    ScheduleOnce(Time::now())));
        }

        void onNewData(DataPackage data) 
        {
            const T& value = this->mutator.getPackagePayload(data);

            // Create a new copy of the data so we can take ownership.
            ChannelData<T> channelData = ChannelData<T>::fromCopy(value);
            this->invokeCallback(channelData);
        }
    };
}