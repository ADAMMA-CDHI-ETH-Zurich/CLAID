#include "RunnableDispatcherThread/RunnableDispatcherThread.hpp"
#include <functional>

#include "RunnableDispatcherThread/Subscriber.hpp"
#include "ChannelRead.hpp"
#include "mutex"
#include <iostream>

namespace portaible
{
    // forward declaration
    template<typename T>
    class TypedChannel;

    /*
    1st super fast subscriber

        Some thoughts:
        What happens, if this subscribers is notified two times in a row,
        without the RunnableDispatcher thread having time to handle the first notification?
        I.e. what happens if we do*
        channel.post(data);
        channel.post(data);

        this would result in a
        subscriber.signalNewDataIsAvailable();
        subscriber.signalNewDataIsAvailable();

        Which means that two new runnables are added to the runnableDispatcherThread.

        Then, in the run function of the channelSubscriber, the data with the newest timestamp is queried.
        Two new data samples are available.
        Thus, when the first runnable is executed, the NEWEST data is read already.
        In that case, when the second runnable is executed, it just aborts because
        the last saved timestamp is the same as the current one.
        This may or may not be a good thing. On the one hand, this event is very unlikely
        and most probably would occur during heavy load. In that case, it might be good to skip a
        data point.

        1.1 Changing the behaviour. 
            But what if we want to make sure really each and every data point is processed ? 
            Then we would need to change it a bit:
            When notifyNewDataIsAvailable is called by the calling thread (i.e., the one that posted),
            this thread should also already perform the read and hand it the ChannelRead to the subscriber.
            In that case, we need to make a copy of the ChannelSubscriber everytime (copy the RunnableDispatcherThread)
            and assign the corresponding data to each copy.

            That, btw., is the reason why ChannelSubscriber does not just inherit from FunctionalRunnableWithParams.
            If you think about it, ChannelSubscriber<T> is a Subscriber, and a FunctionalRunnableWithParams<void, ChannelRead<T>>.
            But, FunctionalRunnableWithParams stores the data to call the function() asynchronously internally.
            Here, with ChannelSubscriber, we specifically did it different, to only read the newest data.

    2nd thread safety
    This implementation should be thread safe, but only because of the general design of the channel mechanism.
    Data on channels is stored in a RingBuffer. New data is added to the next slot.
    The RingBuffer itself is thread safe, as it uses mutex.


    
  
    */
    template<typename T>
    class ChannelSubscriber : public Subscriber, public Runnable
    {   
        public: 
            std::function<void(ChannelRead<T>)> function;
            TypedChannel<T>* channel;
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
             }

        public:
            ChannelSubscriber(RunnableDispatcherThread* runnableDispatcherThread,
	                  std::function<void (ChannelRead<T>)> function) : Subscriber(runnableDispatcherThread), function(function)
            {
                this->lastTimeStamp = Time::now();
            }

           void setChannel(TypedChannel<T>* channel)
           {
               this->channel = channel;
           }
            

            Runnable* asRunnable()
            {
                return static_cast<Runnable*>(this);
            }

             
    };  
}