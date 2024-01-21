#ifndef SYNC_UTIL_HH_
#define SYNC_UTIL_HH_


#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>
#include <iostream>

#include "dispatch/core/Logger/Logger.hh"

namespace claid {

template<class T>
class SharedQueue
{
	private:
		std::list<std::shared_ptr<T>> queue;
		std::mutex m;
		std::condition_variable cv;
		bool closed;

	public:
		SharedQueue() : closed(false)
		{ 
				
		}
				
		SharedQueue(int maxSize) : closed(false)
		{

		}

		void close()
		{
			std::unique_lock<std::mutex> lock(m);
			closed = true;
			cv.notify_all();
		}

		bool is_closed()
		{
			std::unique_lock<std::mutex> lock(m);
			return closed;
		}

        void push_front(std::shared_ptr<T> val)  // adds in the front 
        {
            std::unique_lock<std::mutex> lock(m);
			if (closed)
				throw std::logic_error("put to closed channel");

			queue.push_front(val);

			cv.notify_one();
        }

        void push_back(std::shared_ptr<T> val) // adds at the end 
        {
            std::unique_lock<std::mutex> lock(m);
			if (closed)
				throw std::logic_error("put to closed channel");

			Logger::logInfo("Pushing back %lu", this);
			queue.push_back(val);
			Logger::logInfo("Pushed back");

			cv.notify_one();
			Logger::logInfo("Notified %lu", this);
        }
        
        std::shared_ptr<T> pop_front()  // returns null_ptr if empty 
        {
            std::unique_lock<std::mutex> lock(m);
			
            // Have to account for spurious wakeups?
			cv.wait(lock, [&]() { return closed || !queue.empty(); });

			if (queue.empty())
				return nullptr;

			std::shared_ptr<T> out = queue.front();
			queue.pop_front();

            return out;
        }

		std::shared_ptr<T> interruptable_pop_front()  // returns null_ptr if empty 
        {
			bool empty = queue.empty();
            std::unique_lock<std::mutex> lock(m);
			
            // Have to account for spurious wakeups?
			Logger::logInfo("Queue lock %lu", this);

			if(empty)
			{
				Logger::logInfo("Queue locked %lu", this);
				cv.wait(lock);
			}
			Logger::logInfo("Shared queue wakup %lu", this);
			if (queue.empty() || closed)
				return nullptr;

			std::shared_ptr<T> out = queue.front();
			this->queue.pop_front();

            return out;
        }

		void interruptOnce()
		{
			Logger::logInfo("Queue wakeup %lu", this);
			cv.notify_one();
		}

		size_t size()
		{
			std::unique_lock<std::mutex> lock(m);
			return queue.size();
		}
};

}  // namespace claid  

#endif  // SYNC_UTIL_HH
