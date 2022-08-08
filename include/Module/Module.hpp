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
            bool baseModuleInitialized = false;
            
            bool isSafeToAccessChannels()
            {
                // If runnableDispatcherThread is invalid, subscribing to channels 
                // might result in segfaults (if data is posted to the channel and a subscriber callback
                // is triggered).
                return this->runnableDispatcherThread.get() != nullptr && this->baseModuleInitialized;
            } 

        
            

        protected:
            ChannelManager* channelManager;
            std::shared_ptr<RunnableDispatcherThread> runnableDispatcherThread = nullptr;

            std::vector<DispatcherThreadTimer*> timers;

            std::string id;


            void initializeInternal()
            {
                Logger::printfln("Starting module %s", getDataTypeRTTIString(*this).c_str());
                this->baseModuleInitialized = true;
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

            void verifySafeAccessToChannels(const std::string& channelID)
            {
                if(!this->isSafeToAccessChannels())
                {
                    PORTAIBLE_THROW(Exception, "Error, Module of Class " << getDataTypeRTTIString(*this) << " tried to publish or subscribe to channel with ID \"" << channelID << "\", while the Module has not yet been initialized."
                    "Please only use publish and subscribe in the initialize function, or anytime after initialization was finished successfully (i.e. Module has been started and initialized).");
                }
            }

                       
            virtual void initialize() = 0;

            PropertyReflector propertyReflector;

        public:
            BaseModule()
            {
                if(this->runnableDispatcherThread.get() != nullptr)
                {   
                    PORTAIBLE_THROW(Exception, "Error! Constructor of BaseModule called, but RunnableDispatcherThread is not null!")
                }
                this->runnableDispatcherThread = std::shared_ptr<RunnableDispatcherThread>(new RunnableDispatcherThread());
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
                // If this thread is runnable dispatcher thread, waiting would result in a deadlock.
                if(std::this_thread::get_id() == this->runnableDispatcherThread->getThreadID())
                {
                    PORTAIBLE_THROW(Exception, "Error! Function waitForInitialization() of Module " << getDataTypeRTTIString(*this) << "\n"
                    << " was called from the same thread the Module runs in. This results in an unresolvable deadlock."
                    << "This might happen if you call waitForInitializiation() in any of the Modules functions, or "
                    << "if you use forkSubModuleInThread to create a local SubModule and then call waitForInitialization() on that SubModule "
                    << "(remember the forked SubModule runs on the same thread as the parent Module that forked it).");
                }

                while(!this->initialized)
                {
                    
                }
            }

            void startModule()
            {
                if(this->runnableDispatcherThread.get() == nullptr)
                {
                    PORTAIBLE_THROW(Exception, "Error! startMoudle was called while with RunnableDispatcherThread not set!");
                }

                // PropertyReflector will initialize all members and properties to their default values,
                // if any have been specified.
                this->propertyReflector.reflect(this->id, *this);

             

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

                this->isRunning = true;

                this->runnableDispatcherThread->addRunnable(functionRunnable);

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

                // Should we ? That means when restarting the module,
                // timers need to be registered again.. Which seems fine though.
                this->timers.clear();

                this->isRunning = false;
            }

            // UNIQUE IN THE CURRENT RUNTIME!
            // NOT unique if module RunTimes are connected. In that case, the identifier might not be unique between RunTimes.
            uint64_t getUniqueIdentifier()
            {
                // Use the ID of the dispatcher thread as unique identifier.
                // That mans that submodules spawned by spawnSubModuleInSameThread also have the same identifier!
                // See comment above: only unique in the local RunTime.
                
                static_assert(sizeof(uint64_t) >= sizeof(intptr_t), "Error, type uint64_t is smaller than type intptr_t."
                "Can not store pointers/adresses in objects of type uint64_t with the current compiler, settings or target architecture.");

                return reinterpret_cast<uint64_t>(this->runnableDispatcherThread.get());
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
    // Please note: By spawning a submodule in the same thread,
    // the submodule will have the same unique identifier as the original module.
    class SubModule : public BaseModule
    {
        // SubModuleFactory is allowed to call setDispatcherThread.
        friend class SubModuleFactory;
        private:
            std::shared_ptr<ChannelManager> channelManagerSharedPtr;
            
            void setDispatcherThread(std::shared_ptr<RunnableDispatcherThread> thread)
            {
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
        // Helper factory that allows to access the private constructor of SubModule (as it is a friend class of SubModule).
        // Please note: By spawning a submodule in the same thread,
        // the submodule will have the same unique identifier as the original module.
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

            // Please note: By spawning a submodule in the same thread,
            // the submodule will have the same unique identifier as the original module.
            template<typename SubModuleType, typename... arguments>
            SubModuleType* forkSubModuleInThread(arguments... args)
            {
                SubModuleType* subModule = SubModuleFactory::spawnSubModuleInThread<SubModuleType, arguments...>(this->runnableDispatcherThread, args...);
                subModule->startModule();
                // Don't use waitForInitialization()! forkSubModuleInThread is called from the parent Module.
                // The thread of the SubModule will be the same thread as the parent Module.
                // By calling waitForInitialization, we block (shared) thread.
                // startModule inserts a Runnable into this thread, but as the thread is blocked by
                // waitForInitialization, that Runnable can never get executed, thus the subModule
                // will never be initialized.
                // 
                // subModule->waitForInitialization();
                
                this->subModulesInSameThread.push_back(static_cast<SubModule*>(subModule));
                return subModule;
            }

    };

    
    

}


#define REFLECT_BASE(Reflector, Base) Base::reflect(r);