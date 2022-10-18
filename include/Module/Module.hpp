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


#include "ClassFactory/ClassFactory.hpp"
#include "XML/XMLSerializer.hpp"
#include "XML/XMLDeserializer.hpp"
#include "Binary/BinarySerializer.hpp"
#include "Binary/BinaryDeserializer.hpp"


#include "Channel/ChannelManager.hpp"
namespace claid
{
    // Base class for any type of module.
    class BaseModule
    {
        private:
            // Explicitly forbid copying.
            BaseModule(const BaseModule&) = delete;

            bool isRunning = false;
            bool baseModuleInitialized = false;
            bool terminated = false;
            bool initialized = false;

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

            std::map<std::string, DispatcherThreadTimer*> timers;

            std::string id;


            void initializeInternal()
            {
                Logger::printfln("Starting module %s", this->getModuleName().c_str());
                this->baseModuleInitialized = true;
                this->terminated = false;
                this->initialize();
                this->initialized = true;
                Logger::printfln("Module %s unique id: %ul", this->getModuleName().c_str(), this->getUniqueIdentifier());
            }

            void terminateInternal()
            {
                this->terminate();
                this->baseModuleInitialized = false;
                this->initialized = false;
                this->terminated = true;

            }


            std::vector<DispatcherThreadTimer*> timer;


            void registerPeriodicFunction(const std::string& name, std::function<void()> function, size_t periodInMs)
            {
                auto it = this->timers.find(name);

                if(it != this->timers.end())
                {
                    PORTAIBLE_THROW(Exception, "Error in Module " << this->getModuleName() << ". Tried to register function with name \"" << name << "\", but a periodic function with the same name was already registered before.");
                }

                FunctionRunnable<void>* functionRunnable = new FunctionRunnable<void>(function);
                DispatcherThreadTimer* dispatcherThreadTimer = new DispatcherThreadTimer(this->runnableDispatcherThread, static_cast<Runnable*>(functionRunnable), periodInMs);
                dispatcherThreadTimer->start();
                this->timers.insert(std::make_pair(name, dispatcherThreadTimer));
            }

            template<typename Class>
            void registerPeriodicFunction(const std::string& name, void (Class::* f)(), Class* obj, size_t periodInMs)
            {
                std::function<void()> function = std::bind(f, obj);
                this->registerPeriodicFunction(name, function, periodInMs);
            }

            void unregisterPeriodicFunction(const std::string& name)
            {
                auto it = this->timers.find(name);

                if(it == this->timers.end())
                {
                    PORTAIBLE_THROW(Exception, "Error, tried to unregister periodic function \"" << name << "\" in Module << \"" << this->getModuleName() << "\", but function was not found in list of registered functions."
                    << "Was a function with this name ever registered before?" );
                }

                it->second->stop();
                this->timers.erase(it);
                
                delete it->second;
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
                    PORTAIBLE_THROW(Exception, "Error, Module of Class " << this->getModuleName() << " tried to publish or subscribe to channel with ID \"" << channelID << "\", while the Module has not yet been initialized."
                    "Please only use publish and subscribe in the initialize function, or anytime after initialization was finished successfully (i.e. Module has been started and initialized).");
                }
            }

            
                       
            virtual void initialize() = 0;
            virtual void terminate() = 0;

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

            virtual ~BaseModule()
            {
                Logger::printfln("Module %s destructor", this->getModuleName().c_str());
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
                    PORTAIBLE_THROW(Exception, "Error! Function waitForInitialization() of Module " << this->getModuleName() << "\n"
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
                    PORTAIBLE_THROW(Exception, "Error! startModule was called while with RunnableDispatcherThread not set!");
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

            void stopModule(bool isForkedSubModule = false)
            {
                if(!this->isRunning || this->terminated)
                {
                    return;
                }

                std::function<void ()> terminateFunc = std::bind(&BaseModule::terminateInternal, this);
                FunctionRunnable<void>* functionRunnable = new FunctionRunnable<void>(terminateFunc);
                functionRunnable->deleteAfterRun = true;
                this->runnableDispatcherThread->addRunnable(functionRunnable);

                // Wait for termination
                while(!this->terminated)
                {

                }

                if(!isForkedSubModule)
                {
                    this->runnableDispatcherThread->stop();
                    this->runnableDispatcherThread->join();
                }
                
                
                for(auto it : this->timers)
                {
                    // Will block until the timer is stopped.
                    it.second->stop();
                    delete it.second;
                }

                // Should we ? That means when restarting the module,
                // timers need to be registered again.. Which seems fine though.
                this->timers.clear();

                this->isRunning = false;
                Logger::printfln("Module %s has been terminated.", this->getModuleName().c_str());
            }

            // UNIQUE IN THE CURRENT RUNTIME!
            // NOT unique if module RunTimes are connected. In that case, the identifier might not be unique between RunTimes.
            // If a SubModule is *forked* (if forkSubModuleInThisThread is used), that SubModule has THE SAME unique identifier as the forking Module.
            uint64_t getUniqueIdentifier()
            {
                // Use the ID of the dispatcher thread as unique identifier.
                // That mans that submodules spawned by spawnSubModuleInSameThread also have the same identifier!
                // See comment above: only unique in the local RunTime.
                
                // This probably is only false on 128bit systems or what? lol 
                // Or in weird compilers... Looking at you MSVC
                static_assert(sizeof(uint64_t) >= sizeof(intptr_t), "Error, type uint64_t is smaller than type intptr_t."
                "Can not store pointers/adresses in objects of type uint64_t with the current compiler, settings or target architecture.");

                return reinterpret_cast<uint64_t>(this->runnableDispatcherThread.get());
            }

            std::string getModuleName()
            {
                return TypeChecking::getCompilerSpecificRunTimeNameOfObject(*this);   
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

            virtual void terminate()
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
        DECLARE_CLASS_FACTORY(Module)
        DECLARE_POLYMORPHIC_REFLECTOR(Module, claid::XMLSerializer, XMLSerializer)
        DECLARE_POLYMORPHIC_REFLECTOR(Module, claid::XMLDeserializer, XMLDeserializer)
        DECLARE_POLYMORPHIC_REFLECTOR(Module, claid::BinarySerializer, BinarySerializer)
        DECLARE_POLYMORPHIC_REFLECTOR(Module, claid::BinaryDeserializer, BinaryDeserializer)

        public:
            Module();

            virtual ~Module()
            {
                for(SubModule* subModule : this->subModulesInSameThread)
                {
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

            virtual void terminate()
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

            void joinAndRemoveSubModule(SubModule* subModule)
            {

                auto it = std::find(this->subModulesInSameThread.begin(), this->subModulesInSameThread.end(), subModule);

                if(it == this->subModulesInSameThread.end())
                {
                    PORTAIBLE_THROW(Exception, "Error! joinAndRemoveSubModule was called with a SubModule pointer that is not known. Either this SubModule has been removed manually, "
                    "or it was not created by using forkSubModuleInThread in the first place. The function joinAndRemoveSubModule can only be used for forked SubModules!");
                }
                else
                {
                    subModule->stopModule(true);
                    delete subModule;
                    this->subModulesInSameThread.erase(it);
                }

            }

    };

    
    

}

