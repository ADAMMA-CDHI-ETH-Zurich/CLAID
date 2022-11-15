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
            bool running = false;
        
        
            std::thread* loadingThread = nullptr;
            std::thread* separateProcessThread = nullptr;

            std::vector<Module*> instantiateModulesFromRootXMLNode(std::shared_ptr<XMLNode> node);
            void addModules(std::vector<Module*> modules);
            void startModules(std::vector<Module*> modules);
            void loadAndStart();

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

            // See comment in cpp for more details about why we use separate thread here.
            void startLoadingThread();
            void processRunnablesBlocking();

            XMLLoader::XMLLoaderManager loader;


        public:
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

            void addModule(Module* module);
            void connectTo(std::string ip, int port);

            size_t getNumModules();
            size_t getNumChannels();
            const std::string& getChannelNameByIndex(size_t id);

            bool isRunning() const;

            void executeRunnableInRunTimeThread(Runnable* runnable);
            ITCChannel<Runnable*>* getMainRunnablesChannel();

            void loadFromXML(std::string path);
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