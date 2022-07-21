#include "RunTime/RunTime.hpp"

namespace portaible
{
    template<typename T>
    Channel<T> SubModule::subscribeLocal(const std::string& channelID)
    {
        return this->channelManager->subscribe<T>(channelID);
    }

    template<typename T, typename Class>
    Channel<T> SubModule::subscribeLocal(const std::string& channelID,
                    void (Class::*f)(ChannelData<T>), Class* obj)
    {
        std::function<void (ChannelData<T>)> function = std::bind(f, obj, std::placeholders::_1);
        return subscribeLocal(channelID, function); 
    }

    template<typename T>
    Channel<T> SubModule::subscribeLocal(const std::string& channelID, std::function<void (ChannelData<T>)> function)
    {
        // runtime::getChannel(channelID).subscribe()
        ChannelSubscriber<T> channelSubscriber(&this->runnableDispatcherThread, function);
        return this->channelManager->subscribe<T>(channelID, channelSubscriber);
    }

    template<typename T>
    Channel<T> SubModule::subscribeLocal(const std::string& channelID, ChannelSubscriber<T> channelSubscriber)
    {
        return this->channelManager->subscribe(channelID, channelSubscriber);
    }

    template<typename T>
    Channel<T> SubModule::publishLocal(const std::string& channelID)
    {
        return this->channelManager->publish<T>(channelID);
    }
}


namespace portaible
{


template<typename T>
Channel<T> Module::subscribe(const std::string& channelID)
{
    return this->channelManager->subscribe<T>(channelID);
}

template<typename T, typename Class>
Channel<T> Module::subscribe(const std::string& channelID,
                void (Class::*f)(ChannelData<T>), Class* obj)
{
    std::function<void (ChannelData<T>)> function = std::bind(f, obj, std::placeholders::_1);
    return subscribe(channelID, function); 
}

template<typename T>
Channel<T> Module::subscribe(const std::string& channelID, std::function<void (ChannelData<T>)> function)
{
    // runtime::getChannel(channelID).subscribe()
    ChannelSubscriber<T> channelSubscriber(&this->runnableDispatcherThread, function);
    return this->channelManager->subscribe<T>(channelID, channelSubscriber);
}

template<typename T>
Channel<T> Module::subscribe(const std::string& channelID, ChannelSubscriber<T> channelSubscriber)
{
    return this->channelManager->subscribe(channelID, channelSubscriber);
}

template<typename T>
Channel<T> Module::publish(const std::string& channelID)
{
    return this->channelManager->publish<T>(channelID);
}
}

