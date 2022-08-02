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
        private:
            // Explicitly forbid copying.
            BaseModule(const BaseModule&) = delete;

            bool isRunning = false;

        protected:
            ChannelManager* channelManager;
            std::shared_ptr<RunnableDispatcherThread> runnableDispatcherThread = nullptr;

            std::vector<DispatcherThreadTimer*> timers;

            std::string id;


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
                DispatcherThreadTimer* dispatcherThreadTimer = new DispatcherThreadTimer(this->runnableDispatcherThread, static_cast<Runnable*>(functionRunnable), periodInMs);
                dispatcherThreadTimer->start();
                this->timers.push_back(dispatcherThreadTimer);
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
                ChannelSubscriber<T> channelSubscriber(this->runnableDispatcherThread, function);
                return channelSubscriber;
            }

            
            virtual void initialize() = 0;

            PropertyReflector propertyReflector;

        public:
            BaseModule()
            {
            }

          
            template<typename T>
            void reflect(T& r)
            {
              
            }

            template<typename Class, typename... Ts>
            void callLater(void (Class::* f)(Ts...), Class* obj, Ts... params)
            {
                if(this->runnableDispatcherThread.get() == nullptr)
                {
                    PORTAIBLE_THROW(Exception, "Error! callLater was called while module is stopped."
                    "Please only use callLater while the Module is running.");
                }
                //std::function<void(Ts...)> function = std::bind(f, obj, std::placeholders::_1, std::placeholders::_2);

                FunctionRunnableWithParams<void, Ts...>* functionRunnable = new FunctionRunnableWithParams<void, Ts...>();
                functionRunnable->bind(f, obj);
                functionRunnable->setParams(params...);
                functionRunnable->deleteAfterRun = true;
                this->runnableDispatcherThread->addRunnable(functionRunnable);
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

                if(this->runnableDispatcherThread.get() == nullptr)
                {
                    Logger::printfln("Spawning runnable dispatcher thread");
                    this->runnableDispatcherThread = std::shared_ptr<RunnableDispatcherThread>(new RunnableDispatcherThread());
                }

                // If we are a SubModule that was spawned by a Module,
                // we might use the same runnableDispatcherThread.
                // Thus, it might already have been started before.
                if(!this->runnableDispatcherThread->isRunning())
                {
                    this->runnableDispatcherThread->start();
                }
                std::function<void ()> initFunc = std::bind(&BaseModule::initializeInternal, this);
                FunctionRunnable<void>* functionRunnable = new FunctionRunnable<void>(initFunc);

                functionRunnable->deleteAfterRun = true;

                this->runnableDispatcherThread->addRunnable(functionRunnable);

                this->isRunning = true;
            }

            void stopModule()
            {
                if(!this->isRunning)
                {
                    return;
                }

                this->runnableDispatcherThread->stop();
                this->runnableDispatcherThread->join();
                
                for(DispatcherThreadTimer* timer : this->timers)
                {
                    // Will block until the timer is stopped.
                    timer->stop();
                }

                this->isRunning = false;
            }
    };

    // A SubModule can only communicate via local channels (between any two Modules or SubModules) and 
    // has no access to global ones. That means, it has it's own ChannelManager and does not use the global one.
    // Therefore, the publish and subscribe functions are called publishLocal and subscribeLocal, but technically they are
    // Exactly the same as for Module. The only difference between Module and SubModule is the constructor.
    // While in Module, the internal channelManager variable is set to the global ChannelManager,
    // SubModule creates it's own ChannelManager during start.

    // By default, a SubModule always runs in it's own thread.
    // However, using the spawnSubModule function of Module, a SubModule
    // can also be running on the same thread as the parent Module.
    class SubModule : public BaseModule
    {
        // SubModuleFactory is allowed to call setDispatcherThread.
        friend class SubModuleFactory;
        private:
            std::shared_ptr<ChannelManager> channelManagerSharedPtr;
            
            void setDispatcherThread(std::shared_ptr<RunnableDispatcherThread> thread)
            {
                if(this->runnableDispatcherThread != nullptr)
                {
                    PORTAIBLE_THROW(Exception, "Error: setDispatcherThread of SubModule was called AFTER the module has been started."
                    "If dispatcher thread shall be set manually, this needs to be done BEFORE startModule() of the SubModule was called.");
                }
                this->runnableDispatcherThread = thread;
            }

        public:
        
            SubModule() : BaseModule()
            {
                this->channelManager = new ChannelManager;
                // In addition to the normal channelManager ptr, we also create a sharedPtr.
                // This will make sure that if the user creates a copy of a SubModule instance (which should not be possible, but well..),
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

    class SubModuleFactory
    {
        public:
        // We use factory function to make sure that the RemoteConnectedEntity uses
        // a ConnectionModule that was created by itself. I.e.: We make sure we have ownership
        // over the ConnectionModule and it's not exposed to "the outside world".
        template<typename SubModuleType, typename... arguments>
        static SubModuleType* spawnSubModuleInThread(std::shared_ptr<RunnableDispatcherThread> thread, arguments... args)
        {
            SubModuleType* subModule = new SubModuleType(args...);
            subModule->setDispatcherThread(thread);
            return subModule;
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
                for(SubModule* subModule : this->subModulesInSameThread)
                {
                    // Waits for the module to stop.
                    subModule->stopModule();
                    delete subModule;
                }
            }

        private: 
            // SubModules that were created by the Module using spawnSubModule.
            // Those subModules run in the same thread as Module.
            // SubModuels can also be created manually (just by creating a corresponding member variable
            // and running startModule manually), however then they run in their own separate thread.
            std::vector<SubModule*> subModulesInSameThread;

            

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

            template<typename SubModuleType, typename... arguments>
            void spawnSubModule(arguments... args)
            {
                SubModuleType* subModule = SubModuleFactory::spawnSubModuleInThread<SubModuleType, arguments...>(this->runnableDispatcherThread, args...);
                subModule->startModule();
                subModule->waitForInitialization();
                this->subModulesInSameThread.push_back(static_cast<SubModule*>(subModule));
            }

    };

    
    

}


#define REFLECT_BASE(Reflector, Base) \
    Base::reflect(r);