#include "RunTime/RunTime.hpp"

namespace portaible
{
    template<typename T>
    Channel<T> SubModule::subscribe(TypedChannel<T>& channel)
    {
        return channel.subscribe();   
    }

    template<typename T, typename Class>
    Channel<T> SubModule::subscribe(TypedChannel<T>& channel,
                void (Class::*f)(ChannelData<T>), Class* obj)
    {
        std::function<void (ChannelData<T>)> function = std::bind(f, obj, std::placeholders::_1);
        return subscribe(channel, function);
    }

    template<typename T>
    Channel<T> SubModule::subscribe(TypedChannel<T>& channel, std::function<void (ChannelData<T>)> function)
    {
        ChannelSubscriber<T> channelSubscriber(&this->runnableDispatcherThread, function);
        return channel.subscribe(channelSubscriber);
    }
    


    template<typename T>
    Channel<T> publish(const std::string& channelID);

    template<typename T>
    void unsubscribe()
    {

    }
}


namespace portaible
{
template<typename T>
inline Channel<T> Module::subscribe(const std::string& channelID)
{
    return PORTAIBLE_RUNTIME->channelManager.subscribe<T>(channelID);
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
    return PORTAIBLE_RUNTIME->channelManager.subscribe<T>(channelID, channelSubscriber);
}
/*
template<typename T, typename Class>
        Channel<T> Module::subscribe(TypedChannel<T>& channel,
                    void (Class::*f)(ChannelData<T>), Class* obj)
{
    std::function<void (ChannelData<T>)> function = std::bind(f, obj, std::placeholders::_1);
    return subscribe(channel, function);
}

template<typename T>
Channel<T> Module::subscribe(TypedChannel<T>& channel, std::function<void (ChannelData<T>)> function)
{
    ChannelSubscriber<T> channelSubscriber(&this->runnableDispatcherThread, function);
    return channel.subscribe(channelSubscriber);
}*/

template<typename T>
Channel<T> Module::publish(const std::string& channelID)
{
    return PORTAIBLE_RUNTIME->channelManager.publish<T>(channelID);
}
}

