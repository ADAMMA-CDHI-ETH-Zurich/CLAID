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

#include "Channel/ChannelManager.hpp"
namespace portaible
{
    // Base class for any type of module.
    class BaseModule
    {
        protected:
            ChannelManager* channelManager;

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


            template<typename T, typename Class>
            ChannelSubscriber<T> makeSubscriber(void (Class::*f)(ChannelData<T>), Class* obj)         
            {
                std::function<void (ChannelData<T>)> function = std::bind(f, obj, std::placeholders::_1);
                return makeSubscriber(function);
            }   

            template<typename T>
            ChannelSubscriber<T> makeSubscriber(std::function<void (ChannelData<T>)> function)
            {
                // runtime::getChannel(channelID).subscribe()
                ChannelSubscriber<T> channelSubscriber(&this->runnableDispatcherThread, function);
                return channelSubscriber;
            }

            
            virtual void initialize() = 0;

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
            
            void waitForInitialization()
            {
                while(!this->initialized)
                {
                    
                }
            }

            void startModule()
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
    // has no access to global ones. That means, it has it's own ChannelManager and does not use the global one.
    // Therefore, the publish and subscribe functions are called publishLocal and subscribeLocal, but technically they are
    // Exactly the same as for Module. The only difference between Module and SubModule is the constructor.
    // While in Module, the internal channelManager variable is set to the global ChannelManager,
    // SubModule creates it's own ChannelManager during start.
    class SubModule : public BaseModule
    {
        private:
            std::shared_ptr<ChannelManager> channelManagerSharedPtr;
        public:
            SubModule()
            {
                this->channelManager = new ChannelManager;
                // In addition to the normal channelManager ptr, we also create a sharedPtr.
                // This will make sure that if the user creates a copy of a SubModule instance (which he should not do, but well..),
                // this copy can still access the correct channelManager, and the channelManager will be released when all
                // copies of the module have been destroyed.
                // Why not make channelManager a shared_ptr in the first place?
                // Because the Module (class Module) also uses channelManager, however it holds a reference to a global
                // ChannelManager, which shall never be deleted even if all instances of Module have been destroyed.
                this->channelManagerSharedPtr = std::shared_ptr<ChannelManager>(this->channelManager, std::default_delete<ChannelManager>());
            }

            virtual ~SubModule()
            {

            }

            virtual void initialize()
            {

            }

        protected: 
        template<typename T>
        Channel<T> subscribeLocal(const std::string& channelID);

        template<typename T, typename Class>
        Channel<T> subscribeLocal(const std::string& channelID,
                    void (Class::*f)(ChannelData<T>), Class* obj);

        template<typename T>
        Channel<T> subscribeLocal(const std::string& channelID, std::function<void (ChannelData<T>)> function);
        
        template<typename T>
        Channel<T> subscribeLocal(const std::string& channelID, ChannelSubscriber<T> channelSubscriber);

        template<typename T>
        Channel<T> publishLocal(const std::string& channelID);

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
            Module();

            virtual ~Module()
            {

            }

        protected:
            virtual void initialize()
            {

            }

            template<typename T>
            Channel<T> subscribe(const std::string& channelID);

            template<typename T, typename Class>
            Channel<T> subscribe(const std::string& channelID,
                        void (Class::*f)(ChannelData<T>), Class* obj);

            template<typename T>
            Channel<T> subscribe(const std::string& channelID, std::function<void (ChannelData<T>)> function);

            template<typename T>
            Channel<T> subscribe(const std::string& channelID, ChannelSubscriber<T> channelSubscriber);

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