#include "RunTime/RunTime.hpp"

#include "XML/XMLDocument.hpp"
#include "Network/NetworkClientModule.hpp"


namespace claid
{
    std::vector<Module*> RunTime::instantiateModulesFromRootXMLNode(std::shared_ptr<XMLNode> xmlNode)
    {
        printf("Instantiatemodules\n");
        std::vector<Module*> loadedModules;
    
        loadedModules = this->loader.executeAppropriateLoaders(xmlNode);
        
        return loadedModules;
    }

    void RunTime::addModules(std::vector<claid::Module*> modulesToAdd)
    {
        for(Module*& module : modulesToAdd)
        {
            this->addModule(module);
        }
    }

    void RunTime::startModules(std::vector<claid::Module*> modules)
    {
        for(Module*& module : modules)
        {
            module->startModule();

            while(!module->isInitialized())
            {

            }
        }

        // Call postInitialize.
        for(Module*& module : modules)
        {
            module->onAllModulesHaveBeenInitialized();
        }
    }

    void RunTime::loadAndStart()
    {
        // Start all modules that have been added manually before loading XMLs.
        this->startModules(this->modules);
        while(this->running)
        {
            std::shared_ptr<XMLNode> xmlNode;
            
            // Blocking
            loadedXMLConfigsChannel.get(xmlNode);


            std::vector<Module*> loadedModules = this->instantiateModulesFromRootXMLNode(xmlNode);
            this->addModules(loadedModules);
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

    void RunTime::addModule(Module* module)
    {
        return this->modules.push_back(module);
    }

    void RunTime::connectTo(std::string ip, int port)
    {
        claid::Network::NetworkClientModule* clientModule = new claid::Network::NetworkClientModule();
        clientModule->address = ip + std::string(":") + std::to_string(port);
        CLAID_RUNTIME->addModule(clientModule);
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
        printf("tryput\n");
        this->loadedXMLConfigsChannel.put(xmlDocument.getXMLNode());
        printf("Have put\n");
  }
}
