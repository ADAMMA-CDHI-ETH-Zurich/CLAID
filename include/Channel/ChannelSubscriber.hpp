#include "RunnableDispatcherThread/RunnableDispatcherThread.hpp"
#include <functional>

#include "RunnableDispatcherThread/Subscriber.hpp"
#include "ChannelData.hpp"
#include "mutex"
#include <iostream>
#include <deque>

namespace portaible
{
    // forward declaration
    template<typename T>
    class TypedChannel;



    class ChannelSubscriberBase : public Subscriber, public Runnable
    {
        private:
            // ChannelSubscriber has to be copyable, but mutex is non copyable.
            // Thus, create mutex on heap.
            std::shared_ptr<std::mutex> mutex = nullptr;

        protected:
            void lockMutex()
            {
                if(this->mutex.get() == nullptr)
                {
                    this->mutex = std::shared_ptr<std::mutex>(new std::mutex());
                }

                this->mutex->lock();
            }

            void unlockMutex()
            {
                this->mutex->unlock();
            }

        public:
            ChannelSubscriberBase(std::shared_ptr<RunnableDispatcherThread> runnableDispatcherThread) : Subscriber(runnableDispatcherThread)
            {
            
            }
    };

    template<typename T>
    class ChannelSubscriber : public ChannelSubscriberBase
    {   

        private: 
            std::function<void(ChannelData<T>)> function;
            TypedChannel<T>* channel;

            
            std::deque<ChannelData<T>> channelDataQueue;

            


            void run()
            {
                this->lockMutex();
                Logger::printfln("Pop %s %d\n", channel->getChannelID().c_str(), this->channelDataQueue.size());
                ChannelData<T> channelData = this->channelDataQueue.front();
                size_t sizeBeforePop = this->channelDataQueue.size();
                this->channelDataQueue.pop_front();
                this->unlockMutex();

        
                this->function(channelData);
             }

        public:
            ChannelSubscriber(std::shared_ptr<RunnableDispatcherThread> runnableDispatcherThread,
                      std::function<void (ChannelData<T>)> function) : ChannelSubscriberBase(runnableDispatcherThread), function(function)
            {
            }

            void setChannel(TypedChannel<T>* channel)
            {
                this->channel = channel;
            }
            
            void signalNewDataIsAvailable()
            {
                this->lockMutex();
                ChannelData<T> data = channel->read();
                if(data.isValid())
                {
                    Logger::printfln("Queue push back %s %d\n", this->channel->getChannelID().c_str(), this->channelDataQueue.size());
                    this->channelDataQueue.push_back(data);
                    Subscriber::signalNewDataIsAvailable();
                }
                else
                {
                    size_t nddot = 0;
                    std::cout << "not valid\n";
                }
                this->unlockMutex();
            }

            Runnable* asRunnable()
            {
                return static_cast<Runnable*>(this);
            }

             
    };  
}
