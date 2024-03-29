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