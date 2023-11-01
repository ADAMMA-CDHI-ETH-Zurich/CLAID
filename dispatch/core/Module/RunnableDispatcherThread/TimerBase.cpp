/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions
* 
* Authors: Patrick Langer
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
#include "RunnableDispatcherThread/TimerBase.hpp"
#include "Exception/Exception.hpp"
namespace claid
{
    TimerBase::TimerBase()
    {
        
    }
    
    TimerBase::TimerBase(size_t periodInMs) : periodInMs(periodInMs)
    {

    }
    


    void TimerBase::start()
    {
        this->active = true;
        this->thread = std::thread(&TimerBase::runInternal, this);
    }

    void TimerBase::stop()
    {
        std::unique_lock<std::mutex> lock{this->mutex};
        this->active = false;
        lock.unlock();

        cv.notify_one();
        this->thread.join();
    }

    void TimerBase::runInternal()
    {

        auto endTime = std::chrono::system_clock::now();

        std::unique_lock<std::mutex> lock{this->mutex};
        // periodInMs cannot be 0!
        // Otherwise, the thread will never free allocated objects on the heap.
        // E.g., if you do:
        /*
        while (!this->cv.wait_until(lock, endTime += std::chrono::milliseconds(0),
                                 [&]{ return !this->active; }))
        {
            std::string test = "This is a test 123456."
        }
        // This will result in memory leaks, because a string will always get allocated and never freed.
        // Yes, this might be unexpected, as test goes out of scope and the internal character array should be freed automatically.
        // Put if the period is 0, the thread will always be suspended by the cv.wait_until and will actually never free the allocated data.
        // Tested and confirmed as problem on Android and MacOS.
        */
       if(this->periodInMs == 0)
       {
            CLAID_THROW(Exception, "Cannot run timer with period of 0ms. This would result in the thread always being suspended, \n"
            << "after the specified function has been executed. It therefore would not have time to free any allocated data on the heap, \n"
            << "even if an object (e.g., string) goes out of scope. In other words, it would result in memory leaks.\n"
            << "(Confirmed on Android API 27 and MacOS)\n"
            << "See file \"" << __FILE__ << "\" function \"" << __FUNCTION__ << "\" for details.");
       }
        while (!this->cv.wait_until(lock, endTime += std::chrono::milliseconds(periodInMs),
                                 [&]{ return !this->active; }))
        {
            this->run();
        }
    }
}