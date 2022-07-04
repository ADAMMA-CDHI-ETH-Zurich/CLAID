#pragma once


#ifndef _PORTAIBLE_RUNTIME_H_
#  error "Module.h must be included via the RunTime.h. You must not include it directly. Use #include <fw/RunTime.h> instead"
#endif

#include "RunnableDispatcherThread/RunnableDispatcherThread.hpp"
#include "Channel/Channel.hpp"
#include "Channel/ChannelRead.hpp"
#include "RunnableDispatcherThread/DispatcherThreadTimer.hpp"


#include "RunnableDispatcherThread/FunctionRunnable.hpp"
#include "ClassFactory/ClassFactory.hpp"
#include "PolymorphicReflector/PolymorphicReflector.hpp"
#include "Reflection/PropertyReflector.hpp"
namespace portaible
{
    class Module
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



            PropertyReflector propertyReflector;

        public:
            template<typename T>
            void reflect(T& r)
            {
              
            }

            void setID(const std::string& id)
            {
                this->id = id;
            }

            virtual void initialize()
            {

            }
            
            bool isInitialized()
            {
                return this->initialized;
            }
            
            template<typename T>
            Channel<T> subscribe(const std::string& channelID);
            template<typename T, typename Class>
            Channel<T> subscribe(const std::string& channelID,
	                     void (Class::*f)(ChannelRead<T>), Class* obj);

            template<typename T>
            Channel<T> subscribe(const std::string& channelID, std::function<void (ChannelRead<T>)> function);

            template<typename T>
            Channel<T> publish(const std::string& channelID);

            template<typename T>
            void unsubscribe()
            {

            }

            void start()
            {
                // PropertyReflector will initialize all members and properties to their default values,
                // if any have been specified.
                this->propertyReflector.reflect(this->id, *this);


                this->runnableDispatcherThread.start();

                std::function<void ()> initFunc = std::bind(&Module::initializeInternal, this);
                FunctionRunnable<void>* functionRunnable = new FunctionRunnable<void>(initFunc);

                functionRunnable->deleteAfterRun = false;

                this->runnableDispatcherThread.addRunnable(functionRunnable);
            }
    };




}

#define PORTAIBLE_MODULE(className)\
 DECLARE_CLASS_FACTORY(className)\
 DECLARE_POLYMORPHIC_REFLECTOR(className, portaible::XMLDeserializer)\

#define PORTAIBLE_SERIALIZATION(className)\
REGISTER_TO_CLASS_FACTORY(className) \
REGISTER_POLYMORPHIC_REFLECTOR(className, portaible::XMLDeserializer) \

#define REFLECT_BASE(Reflector, Base) \
    Base::reflect(r);