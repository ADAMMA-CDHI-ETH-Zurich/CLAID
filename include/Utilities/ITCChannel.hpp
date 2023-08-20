#pragma once
#ifndef ITCCHANNEL_H
#define ITCCHANNEL_H

// Channel implementation from: https://st.xorian.net/blog/2012/08/go-style-channel-in-c/

#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>


template<class item>
class ITCChannel
{
	private:
		std::list<item> queue;
		std::mutex m;
		std::condition_variable cv;
		bool closed;
		int maxSize = -1;

	public:
		ITCChannel() : closed(false), maxSize(-1)
		{ 
				
		}
				
		ITCChannel(int maxSize) : closed(false), maxSize(maxSize)
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

		void put(const item &i) {
			std::unique_lock<std::mutex> lock(m);
			if (closed)
				throw std::logic_error("put to closed channel");

			
			queue.push_back(i);

			if (maxSize != -1)
			{
				if (queue.size()  > this->maxSize)
				{
					queue.pop_front();
				}
			}

			cv.notify_one();
		}

		bool get(item &out, bool wait = true)
		{
			std::unique_lock<std::mutex> lock(m);
			if (wait)
				cv.wait(lock, [&]() { return closed || !queue.empty(); });
			if (queue.empty())
				return false;
			out = queue.front();
			queue.pop_front();
			return true;
		}

		size_t size()
		{
			std::unique_lock<std::mutex> lock(m);
			return queue.size();
		}
};
	


#endif