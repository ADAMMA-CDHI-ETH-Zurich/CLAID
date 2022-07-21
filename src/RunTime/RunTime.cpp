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

}
