#include "RunTime/RunTime.hpp"

namespace portaible
{
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

    void RunTime::start()
    {
        if(this->running)
        {
            PORTAIBLE_THROW(Exception, "Error in RunTime::start(), start was called twice !");
        }
        this->startModules();
        this->running = true;

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


}
