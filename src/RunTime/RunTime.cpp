#include "RunTime/RunTime.hpp"

#include "XML/XMLDocument.hpp"
#include "XMLModules/XMLReceiverModule.hpp"
#include "Network/NetworkClientModule.hpp"
#include "Network/NetworkServerModule.hpp"


namespace claid
{
    RunTime::RunTime()
    {
        this->addHiddenNamespace("CLAID/LOCAL/");
    }

    std::vector<Module*> RunTime::instantiateModulesFromRootXMLNode(std::shared_ptr<XMLNode> xmlNode)
    {
        std::vector<Module*> loadedModules;
    
        loadedModules = this->loader.executeAppropriateLoaders(xmlNode);
        
        return loadedModules;
    }

    void RunTime::insertModules(std::vector<claid::Module*> modulesToAdd)
    {
        for(Module*& module : modulesToAdd)
        {
            this->modules.push_back(module);
        }
    }

    void RunTime::startModules(std::vector<claid::Module*> modulesToStart)
    {
        for(Module*& module : modulesToStart)
        {
            module->startModule();

            while(!module->isInitialized())
            {

            }
        }

        // Call postInitialize.
        for(Module*& module : modulesToStart)
        {
            module->onAllModulesHaveBeenInitialized();
        }
    }

    void RunTime::loadAndStart()
    {
        // Start all modules that have been added manually usingAddModule before loading XMLs.
        // If a module is added using addModule after CLAID has started, that module will also be
        // started automatically.
        this->startModules(this->modules);
        
        
        while(this->running)
        {
            std::shared_ptr<XMLNode> xmlNode;
            
            // Blocking
            loadedXMLConfigsChannel.get(xmlNode);


            std::vector<Module*> loadedModules = this->instantiateModulesFromRootXMLNode(xmlNode);
            this->insertModules(loadedModules);
            this->startModules(loadedModules);
        }
    }

    void RunTime::startLoadingThread()
    {
        if(this->running)
        {
            CLAID_THROW(Exception, "Error in RunTime::start(), start was called twice !");
        }
        printf("Starting loading thread\n");
        // Some Modules, such as PythonModules, need to execute functions on the main thread of the RunTime during loading and initialization.
        // To do so, they insert Runnables to the runnablesChannel of the RunTime, which is processed below (see while loop).
        // Therefore, a deadlock would occur, if we call startModules from this thread, and the Modules would need to call functions
        // on the main thread during loading (e.g., creating a new instance of a PythonModule object from a python file) or in their initialize function.
        // Therefore, we call loadAndStart in a separate thread.
        this->running = true;
        this->loadingThread = new std::thread(&RunTime::loadAndStart, this);

        // Do NOT join thread here! Some Modules might require to execute code on the main thread before being able to finish the serialization (e.g., PythonModules).
    }

    void RunTime::processRunnablesBlocking()
    {
        // Blocking the thread the RunTime was started in.
        // If necessary, it is possible to call functions on the
        // RunTime thread by adding a runnable to the runnablesChannel (note: ITCChannel, not CLAID::Channel).
        // This might be required to implement functionalities that need to be run on the same thread as the RunTime.
        // E.g., some frameworks rely on functions to be executed on the main thread of an application (e.g., GUI operations).
        // Most likely though, this functionality probably is not required. We just provide it for edge cases.
        // In whole CLAID, only the PythonInterpreter uses this functionality, to ensure that calls to Python are
        // always executed on the same thread as the RunTime. This allows to easily use OpenCV from PythonModules (OpenCV requires to be run in the main thread of an application).
        while(this->running)
        {
            Runnable* runnable;

            runnablesChannel.get(runnable);

            if(runnable->isValid())
                runnable->run();

            runnable->wasExecuted = true;
            if(runnable->deleteAfterRun)
            {
                delete runnable;
            }
        }     
        printf("join\n");
        this->loadingThread->join();        
    }

    void RunTime::start()
    {
        startLoadingThread();
        processRunnablesBlocking();          
    }

    // If you use this, you need to call process regularly (i.e., polling).
    void RunTime::startNonBlockingWithoutUpdates()
    {
        startLoadingThread();
    }

    void RunTime::startInSeparateThread()
    {
        startLoadingThread();
        this->separateProcessThread = new std::thread(&RunTime::process, this);
    }

    void RunTime::process()
    {
        Runnable* runnable;

        if(runnablesChannel.get(runnable, false))
        {
            if(runnable->isValid())
            runnable->run();

            runnable->wasExecuted = true;
            if(runnable->deleteAfterRun)
            {
                delete runnable;
            }
        }           
    }
    void RunTime::connectTo(std::string ip, int port)
    {
        claid::Network::NetworkClientModule* clientModule = new claid::Network::NetworkClientModule(ip, port);
        CLAID_RUNTIME->addModule(clientModule);
    }

    void RunTime::listenOnPort(int port)
    {
        claid::Network::NetworkServerModule* serverModule = new claid::Network::NetworkServerModule(port);
        CLAID_RUNTIME->addModule(serverModule);
    }

    // Adds an XMLReceiverModule, which allows to receive XML configs from the specified channel.
    // Whenever there is a configuration received on that channel, it will be added to CLAID (using CLAID->loadFromXML).
    void RunTime::addXMLReceiverOnChannel(std::string channelName, bool throwExceptionWhenInvalidConfigIsReceived)
    {   
        claid::XMLReceiverModule* xmlReceiverModule = new claid::XMLReceiverModule(channelName, throwExceptionWhenInvalidConfigIsReceived);
        CLAID_RUNTIME->addModule(xmlReceiverModule);
    }

    void RunTime::addHiddenNamespace(const std::string& ns)
    {
        this->hiddenNamespaces.push_back(ns);   
    }

    size_t RunTime::getNumModules()
    {
        return this->modules.size();
    }

    size_t RunTime::getNumChannels()
    {
        return this->channelManager.getNumChannels();
    }

    const std::string& RunTime::getChannelNameByIndex(size_t id)
    {
        return this->channelManager.getChannelNameByIndex(id);
    }

    bool RunTime::isRunning() const
    {
        return this->running;
    }

    bool RunTime::isInHiddenNamespace(const std::string& channelID) const
    {
        for(const std::string& ns : this->hiddenNamespaces)
        {
            if(channelID.compare(0, ns.size(), ns) == 0)
            {
                return true;
            }
        }
        return false;
    }

    void RunTime::executeRunnableInRunTimeThread(Runnable* runnable)
    {
        this->runnablesChannel.put(runnable);
    }
    
    ITCChannel<Runnable*>* RunTime::getMainRunnablesChannel()
    {
        return &this->runnablesChannel;
    }

    void RunTime::loadFromXML(std::string path)
    {
        claid::XMLDocument xmlDocument;
        if(!xmlDocument.loadFromFile(path))
        {
            CLAID_THROW(Exception, "CLAID::RunTime failed to load from XML file \"" << path << "\".");
        }
        this->loadFromXML(xmlDocument);
    }

    void RunTime::loadFromXML(XMLDocument& xmlDocument)
    {
        this->loadedXMLConfigsChannel.put(xmlDocument.getXMLNode());
    }
}
