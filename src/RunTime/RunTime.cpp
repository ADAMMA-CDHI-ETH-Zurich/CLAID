#include "RunTime/RunTime.hpp"

#include "XML/XMLDocument.hpp"
#include "XMLModules/XMLReceiverModule.hpp"
#include "Network/NetworkClientModule.hpp"
#include "Network/NetworkServerModule.hpp"

#include "Logger/Logger.hpp"
namespace claid
{
    RunTime::RunTime()
    {
        this->addHiddenNamespace("/CLAID/LOCAL/");
    }

    std::vector<Module*> RunTime::instantiateModulesFromXMLNode(std::shared_ptr<XMLNode> xmlNode)
    {
        std::vector<Module*> loadedModules;

        // Evaluates each of the children of xmlNode and executes the loader 
        // that can handle the node tag, if there is any.    
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

    void RunTime::insertModulesLoadedFromXMLConfigs(std::vector<claid::Module*> modulesToAdd)
    {
        for(Module*& module : modulesToAdd)
        {
            this->modulesAddedFromConfigs.push_back(module);
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

        // First, we parse all configs that already have been added.
        // These are configs that were registered before CLAID->start(); was called
        // Hence, we assume that the user wants to start all those modules (+ the ones added using addModule) simultaneously.
        // Therefore, at startup we process all configs already pushed to the channel and only switch to blocking mode after we are done.
        std::shared_ptr<XMLNode> xmlNode;
        Logger::printfln("load and start");

        // Returns false when no more data on the channel.
        while(loadedXMLConfigsChannel.get(xmlNode, false))
        {
            Logger::printfln("loading XML");

            std::vector<Module*> loadedModules = this->instantiateModulesFromXMLNode(xmlNode);

            this->insertModules(loadedModules);
            this->insertModulesLoadedFromXMLConfigs(loadedModules);

        }

        this->startModules(this->modules);


        // From here on, we wait for new configs and parse them as they come in.
        // The modules loaded from the config will be started immediately.
        
        while(this->running)
        {
            
            // Blocking
            loadedXMLConfigsChannel.get(xmlNode);

            parseXMLAndStartModules(xmlNode);

        }
    }

    std::vector<Module*> RunTime::parseXMLAndStartModules(std::shared_ptr<XMLNode> xmlNode)
    {
        std::vector<Module*> loadedModules = this->instantiateModulesFromXMLNode(xmlNode);
        this->insertModules(loadedModules);
        this->insertModulesLoadedFromXMLConfigs(loadedModules);
        this->startModules(loadedModules);

        return loadedModules;
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
            
            processRunnable(runnable);
        }     
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
        this->running = true;
        this->separateProcessThread = new std::thread(&RunTime::processRunnablesBlocking, this);
    }

    void RunTime::process()
    {
        Runnable* runnable;

        if(runnablesChannel.get(runnable, false))
        {
            processRunnable(runnable);
        }           
    }

    void RunTime::processRunnable(Runnable* runnable)
    {
        if(runnable->isValid())
        {
            Logger::printfln("Exceuting runnable in RunTimeThread %d", runnable->catchExceptions);
            if(runnable->catchExceptions)
            {
                try
                {
                    runnable->run();
                }
                catch(std::exception& e)
                {
                    runnable->setException(e.what());
                }
            }
            else
            {
                runnable->run(); 
            }
        }

        runnable->wasExecuted = true;
       
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

    void RunTime::enableLoggingToFile(const std::string& path)
    {
        if(!Logger::enableLoggingToFile(path))
        {
            CLAID_THROW(Exception, "Failed to enable logging to file. File \"" << path << "\" cannot be created.");
        }
    }
            
            
    void RunTime::disableLoggingToFile()
    {
        Logger::disableLoggingToFile();
    }
}
