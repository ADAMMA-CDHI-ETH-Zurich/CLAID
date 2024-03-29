/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Stephan Altmüller
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*         http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
***************************************************************************/

#ifndef SYNC_UTIL_HH_
#define SYNC_UTIL_HH_


#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>
#include <iostream>


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

			queue.push_back(val);

			cv.notify_one();
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

			if(empty)
			{
				cv.wait(lock);
			}
			if (queue.empty() || closed)
				return nullptr;

			std::shared_ptr<T> out = queue.front();
			this->queue.pop_front();

            return out;
        }

		void interruptOnce()
		{
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
