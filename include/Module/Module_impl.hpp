#include "RunTime/RunTime.hpp"

namespace portaible
{
template<typename T>
inline Channel<T> Module::subscribe(const std::string& channelID)
{
    return PORTAIBLE_RUNTIME->channelManager.subscribe<T>(channelID);
}

template<typename T, typename Class>
Channel<T> Module::subscribe(const std::string& channelID,
                void (Class::*f)(ChannelRead<T>), Class* obj)
{
    std::function<void (ChannelRead<T>)> function = std::bind(f, obj, std::placeholders::_1);
    return subscribe(channelID, function); 
}

template<typename T>
Channel<T> Module::subscribe(const std::string& channelID, std::function<void (ChannelRead<T>)> function)
{
    // runtime::getChannel(channelID).subscribe()
    ChannelSubscriber<T> channelSubscriber(&this->runnableDispatcherThread, function);
    return PORTAIBLE_RUNTIME->channelManager.subscribe<T>(channelID, channelSubscriber);
}

template<typename T>
Channel<T> Module::publish(const std::string& channelID)
{
    return PORTAIBLE_RUNTIME->channelManager.publish<T>(channelID);
}
}