#pragma once


#ifndef _CLAID_RUNTIME_H_
#  error "Module.h must be included via the RunTime.h. You must not include it directly. Use #include <fw/RunTime.h> instead"
#endif

#include "RunnableDispatcherThread/RunnableDispatcherThread.hpp"
#include "Channel/Channel.hpp"
#include "Channel/ChannelData.hpp"
#include "RunnableDispatcherThread/FunctionRunnableWithParams.hpp"
#include "RunnableDispatcherThread/ScheduleDescription/ScheduleOnce.hpp"
#include "RunnableDispatcherThread/ScheduleDescription/ScheduleRepeatedIntervall.hpp"

#include "RunnableDispatcherThread/FunctionRunnable.hpp"
#include "ClassFactory/ClassFactory.hpp"
#include "Reflection/ReflectionManager.hpp"
#include "Reflection/PropertyReflector.hpp"


#include "ClassFactory/ClassFactory.hpp"
#include "XML/XMLSerializer.hpp"
#include "XML/XMLDeserializer.hpp"
#include "Binary/BinarySerializer.hpp"
#include "Binary/BinaryDeserializer.hpp"


#include "Channel/ChannelManager.hpp"
#include "Namespace/Namespace.hpp"


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

            std::map<std::string, std::shared_ptr<Runnable>> timers;
            std::vector<Namespace> namespaces;

            std::string id;


            void initializeInternal()
            {
                //Logger::printfln("Starting module %s", this->getModuleName().c_str());
                this->baseModuleInitialized = true;
                this->terminated = false;
                this->initialize();
                this->initialized = true;
            }

            void postInitializeInternal()
            {
                this->postInitialize();
            }

            void terminateInternal()
            {
                this->terminate();
                this->baseModuleInitialized = false;
                this->initialized = false;
                this->terminated = true;

            }



            // sequential execution means that the next execution of this function is only rescheduled if the previous execution was finished.
            void registerPeriodicFunction(const std::string& name, std::function<void()> function, size_t periodInMs, const Time& startTime = Time::invalidTime())
            {   
                if(periodInMs == 0)
                {
                    CLAID_THROW(Exception, "Error in registerPeriodicFunction: Cannot register periodic function \"" << name << "\" (in Module \"" << this->getModuleName() << "\")"
                    << " with a period of 0 milliseconds.\n"
                    << "Period needs to be at least 1ms in order to allow a yield for the thread. Otherwise, this can result in memory leaks on some platforms.");
                }

                auto it = this->timers.find(name);

                if(it != this->timers.end())
                {
                    CLAID_THROW(Exception, "Error in Module " << this->getModuleName() << ". Tried to register function with name \"" << name << "\", but a periodic function with the same name was already registered before.");
                }

                std::shared_ptr<FunctionRunnable<void>> functionRunnable(new FunctionRunnable<void>(function));
                std::shared_ptr<Runnable> runnable = std::static_pointer_cast<Runnable>(functionRunnable);

                Time firstExecutionTime = startTime.isValid() ? 
                    startTime : Time::now() + Duration(std::chrono::milliseconds(periodInMs));

                this->timers.insert(std::make_pair(name, runnable));
                this->runnableDispatcherThread->addRunnable(ScheduledRunnable(runnable, 
                    ScheduleRepeatedIntervall(firstExecutionTime, Duration(std::chrono::milliseconds(periodInMs)))));
            }

            template<typename Class>
            void registerPeriodicFunction(const std::string& name, void (Class::* f)(), Class* obj, size_t periodInMs, const Time& startTime = Time::invalidTime())
            {
                std::function<void()> function = std::bind(f, obj);
                this->registerPeriodicFunction(name, function, periodInMs, startTime);
            }

            void unregisterPeriodicFunction(const std::string& name)
            {
                auto it = this->timers.find(name);

                if(it == this->timers.end())
                {
                    CLAID_THROW(Exception, "Error, tried to unregister periodic function \"" << name << "\" in Module \"" << this->getModuleName() << "\", but function was not found in list of registered functions."
                    << "Was a function with this name ever registered before?");
                }


                it->second->invalidate();
                this->timers.erase(it);
            }

            bool isPeriodicFunctionRegistered(const std::string& name) const
            {
                return this->timers.find(name) != this->timers.end();
            }

            template<typename Class>
            void scheduleFunctionAtTime(const std::string& name, void (Class::* f)(), Class* obj, int hour = 0, int minute = 0, int second = 0, int millisecond = 0)
            {
                if(hour < 0 || hour > 23)
                {
                    CLAID_THROW(claid::Exception, "Error, cannot schedule function \"" << name << "\" at time " << hour << ":" << minute << ":" << second << ":" << millisecond << "\n"
                    << "Hour has to be in range [0, 23], but is " << hour << ".");
                }

                if(minute < 0 || minute > 59)
                {
                    CLAID_THROW(claid::Exception, "Error, cannot schedule function \"" << name << "\" at time " << hour << ":" << minute << ":" << second << ":" << millisecond << "\n"
                    << "Minute has to be in range [0, 59], but is " << minute << ".");
                }

                if(second < 0 || minute > 59)
                {
                    CLAID_THROW(claid::Exception, "Error, cannot schedule function \"" << name << "\" at time " << hour << ":" << minute << ":" << second << ":" << millisecond << "\n"
                    << "Second has to be in range [0, 59], but is " << second << ".");
                }

                if(millisecond < 0 || millisecond > 999)
                {
                    CLAID_THROW(claid::Exception, "Error, cannot schedule function \"" << name << "\" at time " <<hour << ":" << minute << ":" << second << ":" << millisecond << "\n"
                    << "Millisecond has to be in range [0, 999], but is " << millisecond << ".");
                }

                Time scheduledTime = Time::todayAt(hour, minute, second) + Duration(std::chrono::microseconds(millisecond * 1000));
                if(scheduledTime > Time::now())
                {
                    // Time is already passed for today. We schedule it for tomorrow.
                    scheduledTime + Duration::days(1);
                }

                this->scheduleFunctionAtTime(name, f, obj, scheduledTime);
            }

            template<typename Class>
            void scheduleFunctionAtTime(const std::string& name, void (Class::* f)(), Class* obj, const Time& time)
            {
                if(time < Time::now())
                {
                    CLAID_THROW(claid::Exception, "Error, cannot schedule function \"" << name << "\", scheduled time is in the past.");
                }
                
                std::function<void()> function = std::bind(f, obj);
                std::shared_ptr<FunctionRunnable<void>> functionRunnable(new FunctionRunnable<void>(function));
                std::shared_ptr<Runnable> runnable = std::static_pointer_cast<Runnable>(functionRunnable);

                this->runnableDispatcherThread->addRunnable(ScheduledRunnable(runnable, 
                    ScheduleOnce(time)));
            }

            template<typename Class>
            void scheduleFunctionInXDays(const std::string& name, void (Class::* f)(), Class* obj, const size_t days)
            {
                Time time = Time::now() + Duration::days(days);
                this->scheduleFunctionAtTime(name, f, obj, time);
            }

            template<typename Class>
            void scheduleFunctionInXHours(const std::string& name, void (Class::* f)(), Class* obj, const size_t hours)
            {
                Time time = Time::now() + Duration::hours(hours);
                this->scheduleFunctionAtTime(name, f, obj, time);
            }

            template<typename Class>
            void scheduleFunctionInXMinutes(const std::string& name, void (Class::* f)(), Class* obj, const size_t minutes)
            {
                Time time = Time::now() + Duration::minutes(minutes);
                this->scheduleFunctionAtTime(name, f, obj, time);
            }

            template<typename Class>
            void scheduleFunctionInXSeconds(const std::string& name, void (Class::* f)(), Class* obj, const size_t seconds)
            {
                Time time = Time::now() + Duration::seconds(seconds);
                this->scheduleFunctionAtTime(name, f, obj, time);
            }

            template<typename Class>
            void scheduleFunctionInXMilliSeconds(const std::string& name, void (Class::* f)(), Class* obj, const size_t milliseconds)
            {
                Time time = Time::now() + Duration::milliseconds(milliseconds);
                this->scheduleFunctionAtTime(name, f, obj, time);
            }
            
            template<typename Class>
            void scheduleFunctionAfterDuration(const std::string& name, void (Class::* f)(), Class* obj, const Duration duration)
            {
                Time time = Time::now() + duration;
                this->scheduleFunctionAtTime(name, f, obj, time);
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
                    CLAID_THROW(Exception, "Error, Module of Class " << this->getModuleName() << " tried to publish or subscribe to channel with ID \"" << channelID << "\", while the Module has not yet been initialized."
                    "Please only use publish and subscribe in the initialize function, or anytime after initialization was finished successfully (i.e. Module has been started and initialized).");
                }
            }

            
                       
            // This function will be called when Module is started.
            virtual void initialize() = 0;

            // This function will be called after all Modules have been initialized.
            virtual void postInitialize() = 0;

            // This function will be called when module is shut down.
            virtual void terminate() = 0;


            PropertyReflector propertyReflector;

            protected:
                

        public:
            BaseModule()
            {
                this->initialized = false;
                if(this->runnableDispatcherThread.get() != nullptr)
                {   
                    CLAID_THROW(Exception, "Error! Constructor of BaseModule called, but RunnableDispatcherThread is not null!")
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
            void callInModuleThread(void (Class::* f)(Ts...), Class* obj, Ts... params)
            {
                if(this->runnableDispatcherThread.get() == nullptr)
                {
                    CLAID_THROW(Exception, "Error! callInModuleThread was called while module is stopped."
                    "Please only use callInModuleThread while the Module is running.");
                }
                //std::function<void(Ts...)> function = std::bind(f, obj, std::placeholders::_1, std::placeholders::_2);

                std::shared_ptr<FunctionRunnableWithParams<void, Ts...>> functionRunnable(new FunctionRunnableWithParams<void, Ts...>());
                functionRunnable->bind(f, obj);
                functionRunnable->setParams(params...);
                this->runnableDispatcherThread->addRunnable(
                    ScheduledRunnable(std::static_pointer_cast<Runnable>(functionRunnable), ScheduleOnce(Time::now())));
            }

            void setID(const std::string& id)
            {
                this->id = id;
            }

            bool isInitialized() const
            {
                return this->initialized;
            }

            bool isModuleRunning() const
            {
                return this->isRunning;
            }
            
            void waitForInitialization()
            {
                Logger::printfln("WaitForInitialization called %d", this->initialized);
                // If this thread is runnable dispatcher thread, waiting would result in a deadlock.
                if(std::this_thread::get_id() == this->runnableDispatcherThread->getThreadID())
                {
                    CLAID_THROW(Exception, "Error! Function waitForInitialization() of Module " << this->getModuleName() << "\n"
                    << " was called from the same thread the Module runs in. This results in an unresolvable deadlock."
                    << "This might happen if you call waitForInitializiation() in any of the Modules functions, or "
                    << "if you use forkSubModuleInThread to create a local SubModule and then call waitForInitialization() on that SubModule "
                    << "(remember the forked SubModule runs on the same thread as the parent Module that forked it).");
                }

                while(!this->initialized)
                {
                    Logger::printfln("WaitForIniti %s", this->getModuleName().c_str());
                    std::this_thread::sleep_for(std::chrono::milliseconds(20));
                }
            }

            void startModule()
            {
                if(this->isRunning)
                {
                    CLAID_THROW(Exception, "Error, startModule was called twice on Module \"" << this->getModuleName() << "\".");
                }

                if(this->runnableDispatcherThread.get() == nullptr)
                {
                    CLAID_THROW(Exception, "Error! startModule was called while with RunnableDispatcherThread not set!");
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
                std::shared_ptr<void> functionRunnable(new FunctionRunnable<void>(initFunc));

                this->isRunning = true;

                this->runnableDispatcherThread->addRunnable(
                        ScheduledRunnable(std::static_pointer_cast<Runnable>(functionRunnable), ScheduleOnce(Time::now())));
            }

            void stopModule(bool isForkedSubModule = false)
            {
                Logger::printfln("Module %s termination requested.", this->getModuleName().c_str());

                if(!this->isRunning || this->terminated)
                {
                    return;
                }

                

                for(auto it : this->timers)
                {
                    // Will block until the timer is stopped.
                    it.second->invalidate();
                }

                // Should we ? That means when restarting the module,
                // timers need to be registered again.. Which seems fine though.
                this->timers.clear();

                std::function<void ()> terminateFunc = std::bind(&BaseModule::terminateInternal, this);
                std::shared_ptr<FunctionRunnable<void>> functionRunnable(new FunctionRunnable<void>(terminateFunc));

                if(!isForkedSubModule)
                {
                    // This means that this runnable will be the last runnable ever executed by this thread.
                    // Even if there are other runnables in the queue, they will not be executed.
                    // This is necessary, because usually modules run clean up code in their terminate function.
                    functionRunnable->stopDispatcherAfterThisRunnable = true;
                }

                this->runnableDispatcherThread->addRunnable(
                        ScheduledRunnable(std::static_pointer_cast<Runnable>(functionRunnable), ScheduleOnce(Time::now())));

                // Wait for termination
                while(!this->terminated)
                {
                }

                if(!isForkedSubModule)
                {
                    this->runnableDispatcherThread->join();
                }
                
                
                

                this->isRunning = false;
                Logger::printfln("Module %s %u has been terminated.", this->getModuleName().c_str(), this);
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

            virtual const std::string getModuleName()
            {
                return TypeChecking::getCompilerSpecificRunTimeNameOfObject(*this);   
            }

            void onAllModulesHaveBeenInitialized()
            {
                std::function<void ()> postInitFunc = std::bind(&BaseModule::postInitializeInternal, this);
                std::shared_ptr<FunctionRunnable<void>> functionRunnable(new FunctionRunnable<void>(postInitFunc));


                this->isRunning = true;

                this->runnableDispatcherThread->addRunnable(
                        ScheduledRunnable(std::static_pointer_cast<Runnable>(functionRunnable), ScheduleOnce(Time::now())));
            }

            void prependNamespace(const Namespace& namespaceName)
            {
                this->namespaces.push_back(namespaceName);
            }

            std::string addNamespacesToChannelID(const std::string& channelID) const 
            {
                std::string result = channelID;

                if(result.size() >= 2)
                {
                    // Do not add namespaces to channels that start with //
                    if(result[0] == '/' && result[1] == '/')
                    {
                        return result;
                    }
                }

                for(const Namespace& ns : this->namespaces)
                {
                    ns.prependNamespaceToString(result);
                }

                return result;
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

            virtual void postInitialize()
            {

            }

            virtual void terminate()
            {

            }

            template<typename T>
            void reflect(T& r)
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
        //DECLARE_CLASS_FACTORY(Module)
        // DECLARE_POLYMORPHIC_REFLECTOR(Module, claid::XMLSerializer, XMLSerializer)
        // DECLARE_POLYMORPHIC_REFLECTOR(Module, claid::XMLDeserializer, XMLDeserializer)
        // DECLARE_POLYMORPHIC_REFLECTOR(Module, claid::BinarySerializer, BinarySerializer)
        // DECLARE_POLYMORPHIC_REFLECTOR(Module, claid::BinaryDeserializer, BinaryDeserializer)

        public:
            Module();

            virtual ~Module()
            {
                for(SubModule* subModule : this->subModulesInSameThread)
                {
                    delete subModule;
                }
            }


            template<typename T>
            void reflect(T& r)
            {
              
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

            // This function will be called after all Modules have been initialized.
            virtual void postInitialize()
            {
                
            }

            virtual void terminate()
            {

            }
        public:
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

            void stopSubModule(SubModule* subModule)
            {

                auto it = std::find(this->subModulesInSameThread.begin(), this->subModulesInSameThread.end(), subModule);

                if(it == this->subModulesInSameThread.end())
                {
                    CLAID_THROW(Exception, "Error! joinAndRemoveSubModule was called with a SubModule pointer that is not known. Either this SubModule has been removed manually, "
                    "or it was not created by using forkSubModuleInThread in the first place. The function joinAndRemoveSubModule can only be used for forked SubModules!");
                }
                else
                {
                    subModule->stopModule(true);
                }
            }

            void removeSubModule(SubModule* subModule)
            {
                auto it = std::find(this->subModulesInSameThread.begin(), this->subModulesInSameThread.end(), subModule);

                if(it == this->subModulesInSameThread.end())
                {
                    CLAID_THROW(Exception, "Error! joinAndRemoveSubModule was called with a SubModule pointer that is not known. Either this SubModule has been removed manually, "
                    "or it was not created by using forkSubModuleInThread in the first place. The function joinAndRemoveSubModule can only be used for forked SubModules!");
                }
                else
                {
                    delete subModule;
                    this->subModulesInSameThread.erase(it);
                }
            }

            

    };

    
    

}

