#include "RunnableDispatcherThread/RunnableDispatcherThread.hpp"
#include <functional>

#include "ChannelSubscriberBase.hpp"
#include "ChannelRead.hpp"
#include "mutex"
#include <iostream>

namespace portaible
{
    // forward declaration
    template<typename T>
    class GlobalChannel;

    template<typename T>
    class ChannelSubscriber : public ChannelSubscriberBase, public Runnable
    {   
        public: 
            std::function<void(ChannelRead<T>)> function;
            GlobalChannel<T>* channel;
            Time lastTimeStamp;

            void run()
            {
                ChannelRead<T> channelRead = this->channel->read();

                Time n = channelRead.getTimestamp();

                bool equal = n == this->lastTimeStamp;

                if (!equal)
                {
                    this->lastTimeStamp = n;
                    this->function(channelRead);


                }
                else
                {

                }

            }

        public:
            ChannelSubscriber(RunnableDispatcherThread* runnableDispatcherThread,
	                  std::function<void (ChannelRead<T>)> function) : ChannelSubscriberBase(runnableDispatcherThread), function(function)
            {
                this->lastTimeStamp = Time::now();
            }

           void setChannel(GlobalChannel<T>* channel)
           {
               this->channel = channel;
           }
            

            Runnable* asRunnable()
            {
                return static_cast<Runnable*>(this);
            }

             
    };  
}