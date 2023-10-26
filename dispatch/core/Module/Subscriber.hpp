#pragma once

#include "AbstractSubscriber.hpp"
#include "dispatch/core/Module/RunnableDispatcherThread/RunnableDispatcher.hpp"
#include "dispatch/core/Module/ChannelData.hpp"

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
                            std::make_shared<FunctionRunnableWithParams<void>>();


            functionRunnable->bind(&ChannelSubscriber::run, this);

            std::shared_ptr<Runnable> runnable = std::static_pointer_cast<Runnable>(functionRunnable);
            
            this.callbackDispatcher.addRunnable(runnable);
        }

        void onNewData(DataPackage data) 
        {
            const T& value = this->mutator.getPackagePayload(data);

            // Create a new copy of the data so we can take ownership.
            ChannelData<T> data = ChannelData<T>::fromCopy(value);
            this->invokeCallback(value);
        }
    };
}