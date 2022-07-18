#pragma once


#ifndef _PORTAIBLE_RUNTIME_H_
#  error "Module.h must be included via the RunTime.h. You must not include it directly. Use #include <fw/RunTime.h> instead"
#endif

#include "RunnableDispatcherThread/RunnableDispatcherThread.hpp"
#include "Channel/Channel.hpp"
#include "Channel/ChannelData.hpp"
#include "RunnableDispatcherThread/DispatcherThreadTimer.hpp"
#include "RunnableDispatcherThread/FunctionRunnableWithParams.hpp"

#include "RunnableDispatcherThread/FunctionRunnable.hpp"
#include "ClassFactory/ClassFactory.hpp"
#include "PolymorphicReflector/PolymorphicReflector.hpp"
#include "Reflection/PropertyReflector.hpp"
#include "Serialization/Serialization.hpp"


#define PORTAIBLE_MODULE(className)\
    DECLARE_SERIALIZATION(className)\

#define PORTAIBLE_SERIALIZATION(className)\
    SERIALIZATION(className)\


namespace portaible
{
    // Base class for any type of module.
    class BaseModule
    {
        protected:
            std::string id;

            RunnableDispatcherThread runnableDispatcherThread;

            void initializeInternal()
            {
                this->initialize();
                this->initialized = true;
            }

            bool initialized = false;

            std::vector<DispatcherThreadTimer*> timer;


            void registerPeriodicFunction(std::function<void()> function, size_t periodInMs)
            {
                FunctionRunnable<void>* functionRunnable = new FunctionRunnable<void>(function);
                DispatcherThreadTimer* dispatcherThreadTimer = new DispatcherThreadTimer(&this->runnableDispatcherThread, static_cast<Runnable*>(functionRunnable), periodInMs);
                dispatcherThreadTimer->start();
            }

            template<typename Class>
            void registerPeriodicFunction(void (Class::* f)(), Class* obj, size_t periodInMs)
            {
                std::function<void()> function = std::bind(f, obj);
                this->registerPeriodicFunction(function, periodInMs);
            }


            

            
            virtual void initialize()
            {

            }

            PropertyReflector propertyReflector;

        public:
            template<typename T>
            void reflect(T& r)
            {
              
            }

            template<typename Class, typename... Ts>
            void callLater(void (Class::* f)(Ts...), Class* obj, Ts... params)
            {
                //std::function<void(Ts...)> function = std::bind(f, obj, std::placeholders::_1, std::placeholders::_2);

                FunctionRunnableWithParams<void, Ts...>* functionRunnable = new FunctionRunnableWithParams<void, Ts...>();
                functionRunnable->bind(f, obj);
                functionRunnable->setParams(params...);
                functionRunnable->deleteAfterRun = true;
                this->runnableDispatcherThread.addRunnable(functionRunnable);
            }

            void setID(const std::string& id)
            {
                this->id = id;
            }

           
            
            bool isInitialized()
            {
                return this->initialized;
            }
            


            void start()
            {
                // PropertyReflector will initialize all members and properties to their default values,
                // if any have been specified.
                this->propertyReflector.reflect(this->id, *this);


                this->runnableDispatcherThread.start();

                std::function<void ()> initFunc = std::bind(&BaseModule::initializeInternal, this);
                FunctionRunnable<void>* functionRunnable = new FunctionRunnable<void>(initFunc);

                functionRunnable->deleteAfterRun = true;

                this->runnableDispatcherThread.addRunnable(functionRunnable);
            }
    };

    // A SubModule can only communicate via local channels (between any two Modules or SubModules) and 
    // has no access to global ones (i.e., it cannot use ChannelIDs to subscribe to channels, cannot send data remotely).
    class SubModule : public BaseModule
    {
        public:
            virtual ~SubModule()
            {

            }

        protected: 
        template<typename T>
        Channel<T> subscribe(TypedChannel<T>& channel);

        template<typename T, typename Class>
        Channel<T> subscribe(TypedChannel<T>& channel,
                    void (Class::*f)(ChannelData<T>), Class* obj);

        template<typename T>
        Channel<T> subscribe(TypedChannel<T>& channel, std::function<void (ChannelData<T>)> function);
        


        template<typename T>
        Channel<T> publish(const std::string& channelID);

        template<typename T>
        void unsubscribe()
        {

        }

                
    };

    // A Module can acess all channels globally (within the current process or
    // remote) via ChannelIDs, in contrast to a SubModule.
    class Module : public BaseModule
    {   
        PORTAIBLE_MODULE(Module)

        public:
            virtual ~Module()
            {

            }

        protected:
            template<typename T>
            Channel<T> subscribe(const std::string& channelID);

            template<typename T, typename Class>
            Channel<T> subscribe(const std::string& channelID,
                        void (Class::*f)(ChannelData<T>), Class* obj);

            template<typename T>
            Channel<T> subscribe(const std::string& channelID, std::function<void (ChannelData<T>)> function);

            /* template<typename T, typename Class>
            Channel<T> subscribe(TypedChannel<T>& channel,
                        void (Class::*f)(ChannelData<T>), Class* obj);

            template<typename T>
            Channel<T> subscribe(TypedChannel<T>& channel, std::function<void (ChannelData<T>)> function);*/

            


            template<typename T>
            Channel<T> publish(const std::string& channelID);

            template<typename T>
            void unsubscribe()
            {

            }

    };




}


#define REFLECT_BASE(Reflector, Base) \
    Base::reflect(r);