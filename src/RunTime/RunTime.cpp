#include "RunTime/RunTime.hpp"

#include "XML/XMLDocument.hpp"
#include "Network/NetworkClientModule.hpp"


namespace claid
{
    void RunTime::loadConfigs()
    {
        for(XMLDocument*& xmlDocument : this->xmlConfigs)
        {
            this->loader.executeAppropriateLoaders(xmlDocument->getXMLNode());
        }
    }
    void RunTime::startModules()
    {
        for(Module*& module : this->modules)
        {
            module->startModule();

            while(!module->isInitialized())
            {

            }
        }
    }

    void RunTime::loadAndStart()
    {
        this->loadConfigs();
        this->startModules();
    }

    void RunTime::checkAndStartLoadingThread()
    {
        if(this->running)
        {
            CLAID_THROW(Exception, "Error in RunTime::start(), start was called twice !");
        }

        // Some Modules, such as PythonModules, need to execute functions on the main thread of the RunTime during loading and initialization.
        // To do so, they insert Runnables to the runnablesChannel of the RunTime, which is processed below (see while loop).
        // Therefore, a deadlock would occur, if we call startModules from this thread, and the Modules would need to call functions
        // on the main thread during loading (e.g., creating a new instance of a PythonModule object from a python file) or in their initialize function.
        // Therefore, we call loadAndStart in a separate thread.
        this->startThread = new std::thread(&RunTime::loadAndStart, this);
        this->running = true;

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
        startThread->join();        
    }

    void RunTime::start()
    {
        checkAndStartLoadingThread();
        processRunnablesBlocking();          
    }

    // If you use this, you need to call process regularly (i.e., polling).
    void RunTime::startNonBlocking()
    {
        checkAndStartLoadingThread();
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
        claid::XMLDocument* xmlDocument = new claid::XMLDocument;
        if(!xmlDocument->loadFromFile(path))
        {
            CLAID_THROW(Exception, "CLAID::RunTime failed to load from XML file \"" << path << "\".");
        }
        this->xmlConfigs.push_back(xmlDocument);
    }
}
