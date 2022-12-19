#pragma once
#define _CLAID_RUNTIME_H_

#include <iostream>

#include <stdio.h>
#include "Utilities/Singleton.hpp"
#include "Channel/ChannelManager.hpp"

#include "Module/Module.hpp"
#include "XMLLoader/XMLLoaderManager.hpp"

#include "RunnableDispatcherThread/Runnable.hpp"

#define CLAID_RUNTIME claid::RunTime::getInstance()

namespace claid
{
    // Forward declaration for class XMLDocument
    class XMLDocument;
}

namespace claid
{
    class RunTime : public Singleton<RunTime> 
    {
        private:
            std::vector<Module*> modules;

            // Stores a subset of modules, namely only modules that were instantiated
            // from configs and not added manually.
            std::vector<Module*> modulesAddedFromConfigs;
            bool running = false;
        
        
            std::thread* loadingThread = nullptr;
            std::thread* separateProcessThread = nullptr;

            std::vector<Module*> instantiateModulesFromRootXMLNode(std::shared_ptr<XMLNode> node);
            void insertModules(std::vector<Module*> modules);
            void insertModulesLoadedFromXMLConfigs(std::vector<Module*> modules);
            void startModules(std::vector<Module*> modules);
            void loadAndStart();
            void callPostInitialize(std::vector<Module*> modules);


            // Used to run runnables on the frameworks main thread (i.e., thread the framework was started from).
            // Generally, is not required. It can be benefical, however, if the thread that started the RunTime is the main
            // thread, and some operations shall specifically be performed in that thread. E.g., this sometimes might be necessary
            // for some GUI operations. For example, if a PythonModule wants to use OpenCV, this only works if the PythonInterpreter
            // can execute that function on the main thread.
            ITCChannel<Runnable*> runnablesChannel;

            // Can be used to instantiate modules from configs using a designated thread.
            // Why do we have a separate thread to instantiate modules from configs? 
            // See comment in startLoadingThread ! 
            ITCChannel<std::shared_ptr<XMLNode>> loadedXMLConfigsChannel;

            // Channels in hidden namespaces cannot be observed by other Modules (see ChannelManager::observePublishedChannels, for example).
            // That means that no other Module can find out, if a channel within that namespace exists.
            // Therefore, ConnectionModules (e.g., SocketConnectionModule) will not know that these channels exist, hence will not mirror them to
            // remotely connected RunTimes. 
            // In other words, hidden namespaces allow to keeps Channels *local only*.
            std::vector<std::string> hiddenNamespaces;

            // See comment in cpp for more details about why we use separate thread here.
            void startLoadingThread();
            void processRunnablesBlocking();
            void processRunnable(Runnable* runnable);

            XMLLoader::XMLLoaderManager loader;


        public:
            RunTime();

            ChannelManager channelManager;

            template<typename T>
            void registerLoader(std::string name)
            {
                this->loader.registerLoader<T>(name);
            }

            void test()
            {
            }

            // This is blocking
            void start();

            // This non blocking, as it starts CLAID in a separate thread.
            void startInSeparateThread();

            // This is non blocking, but will not allow other Modules to run 
            // Runnables on the framework thread. This is fine, if the functionality of
            // running Runnables on the framework thread is not required. 
            // Otherwise, process needs to be called manually.
            void startNonBlockingWithoutUpdates();

            // Only required if CLAID was started non blocking.
            void process();

            template<typename ModuleType>
            void addModule(ModuleType* specificModule)
            {
                Module* module = dynamic_cast<Module*>(specificModule);
                if(module == nullptr)
                {
                    std::string className = TypeChecking::getCompilerSpecificRunTimeNameOfObject(specificModule);
                    // The class is not a Module.
                    CLAID_THROW(claid::Exception, "ModuleLoader failed to load Module from XML. Class \"" << className << "\" is not a Module. Did you forget inheriting from claid::Module?");
                }

                // if(this->isRunning() && !module->isModuleRunning())
                // {
                //     // Call initialize and postInitialize.
                //     this->startModules({module});
                // }
                this->modules.push_back(module);
            }

            void connectTo(std::string ip, int port);
            void listenOnPort(int port);

            void addXMLReceiverOnChannel(std::string channelName, bool throwExceptionWhenInvalidConfigIsReceived);

            // Channels in hidden namespaces cannot be observed by other Modules (see ChannelManager::observePublishedChannels, for example).
            // That means that no other Module can find out, if a channel within that namespace exists.
            // Therefore, ConnectionModules (e.g., SocketConnectionModule) will not know that these channels exist, hence will not mirror them to
            // remotely connected RunTimes. 
            // In other words, hidden namespaces allow to keeps Channels *local only*.
            void addHiddenNamespace(const std::string& ns);

            size_t getNumModules();
            size_t getNumChannels();
            const std::string& getChannelNameByIndex(size_t id);

            bool isRunning() const;
            bool isInHiddenNamespace(const std::string& channelID) const;

            void executeRunnableInRunTimeThread(Runnable* runnable);
            ITCChannel<Runnable*>* getMainRunnablesChannel();

            void loadFromXML(std::string path);
            void loadFromXML(XMLDocument& xmlDocument);

            // Don't use this from main thread!
            std::vector<Module*> parseXMLAndStartModules(std::shared_ptr<XMLNode> xmlNode);

            void enableLoggingToFile(const std::string& path);
            void disableLoggingToFile();

    };
}

// Yes, sadly, we need to define this here.
// In Module.hpp would be a cyclic dependency.
#include "Module/ModuleFactory/ModuleFactory.hpp"
#include "Serialization/Serialization.hpp"
#define DECLARE_MODULE(className)\
    DECLARE_CLASS_FACTORY(className)\
    DECLARE_SERIALIZATION_STANDARD_SERIALIZERS_ONLY(className)\
    DECLARE_MODULE_FACTORY(className)\

#define REGISTER_MODULE(className)\
    static_assert(std::is_base_of<claid::BaseModule, className>::value, "Tried to register a class as Module (see above), which did not inherit from BaseModule. Did you forget to inherit from Module or BaseModule. "  ); \
    REGISTER_TO_CLASS_FACTORY(className) \
    REGISTER_SERIALIZATION_STANDARD_SERIALIZERS_ONLY(className)\
    REGISTER_MODULE_FACTORY(className)


#include "Module/Module_impl.hpp"
#include "XMLLoader/XMLLoader_impl.hpp"


#include "Reflection/Reflect.hpp"