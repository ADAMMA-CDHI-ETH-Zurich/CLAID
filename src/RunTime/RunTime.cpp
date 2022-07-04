#include "RunTime/RunTime.hpp"

namespace portaible
{
    void RunTime::startAuthorities()
    {
        for(Authority*& authority : this->authorities)
        {
            authority->start();

            while(!authority->isInitialized())
            {
                
            }
        }
    }

    void RunTime::addAuthority(Authority* authority)
    {
        return this->authorities.push_back(authority);
    }

    size_t RunTime::getNumAuthorities()
    {
        return this->authorities.size();
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
