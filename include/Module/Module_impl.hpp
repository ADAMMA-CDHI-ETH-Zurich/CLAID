#include "RunTime/RunTime.hpp"

namespace claid
{
    template<typename T>
    Channel<T> SubModule::subscribeLocal(const std::string& channelID)
    {
        verifySafeAccessToChannels(channelID);
        return this->channelManager->subscribe<T>(channelID, this->getUniqueIdentifier());
    }

    template<typename T, typename Class>
    Channel<T> SubModule::subscribeLocal(const std::string& channelID,
                    void (Class::*f)(ChannelData<T>), Class* obj)
    {
        verifySafeAccessToChannels(channelID);
        std::function<void (ChannelData<T>)> function = std::bind(f, obj, std::placeholders::_1);
        return subscribeLocal(channelID, function); 
    }

    template<typename T>
    Channel<T> SubModule::subscribeLocal(const std::string& channelID, std::function<void (ChannelData<T>)> function)
    {
        verifySafeAccessToChannels(channelID);
        // runtime::getChannel(channelID).subscribe()
        ChannelSubscriber<T> channelSubscriber(this->runnableDispatcherThread, function);
        return this->channelManager->subscribe<T>(channelID, channelSubscriber, this->getUniqueIdentifier());
    }

    template<typename T>
    Channel<T> SubModule::subscribeLocal(const std::string& channelID, ChannelSubscriber<T> channelSubscriber)
    {
        verifySafeAccessToChannels(channelID);
        return this->channelManager->subscribe(channelID, channelSubscriber, this->getUniqueIdentifier());
    }

    template<typename T>
    Channel<T> SubModule::publishLocal(const std::string& channelID)
    {
        verifySafeAccessToChannels(channelID);
        return this->channelManager->publish<T>(channelID, this->getUniqueIdentifier());
    }
}


namespace claid
{


template<typename T>
Channel<T> Module::subscribe(const std::string& channelID)
{
    verifySafeAccessToChannels(channelID);
    return this->channelManager->subscribe<T>(channelID, this->getUniqueIdentifier());
}

template<typename T, typename Class>
Channel<T> Module::subscribe(const std::string& channelID,
                void (Class::*f)(ChannelData<T>), Class* obj)
{
    verifySafeAccessToChannels(channelID);
    std::function<void (ChannelData<T>)> function = std::bind(f, obj, std::placeholders::_1);
    return subscribe(channelID, function); 
}

template<typename T>
Channel<T> Module::subscribe(const std::string& channelID, std::function<void (ChannelData<T>)> function)
{
    verifySafeAccessToChannels(channelID);
    // runtime::getChannel(channelID).subscribe()
    ChannelSubscriber<T> channelSubscriber(this->runnableDispatcherThread, function);
    return this->channelManager->subscribe<T>(channelID, channelSubscriber, this->getUniqueIdentifier());
}

template<typename T>
Channel<T> Module::subscribe(const std::string& channelID, ChannelSubscriber<T> channelSubscriber)
{
    verifySafeAccessToChannels(channelID);
    return this->channelManager->subscribe(channelID, channelSubscriber, this->getUniqueIdentifier());
}

template<typename T>
Channel<T> Module::publish(const std::string& channelID)
{
    verifySafeAccessToChannels(channelID);
    return this->channelManager->publish<T>(channelID, this->getUniqueIdentifier());
}
}


