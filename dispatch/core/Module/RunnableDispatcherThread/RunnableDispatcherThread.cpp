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
#include "RunnableDispatcherThread/RunnableDispatcherThread.hpp"
#include "RunnableDispatcherThread/DummyRunnable.hpp"


namespace claid
{
    void RunnableDispatcherThread::start()
    {
        if(this->active)
        {
            return;
        }
        this->runnableDispatcher.init();

        this->active = true;
        this->thread = std::thread(&RunnableDispatcherThread::run, this);
    }

    void RunnableDispatcherThread::stop()
    {
        if(!this->active)
        {
            return;
        }
        this->active = false;
        this->runnableDispatcher.stop();
    }

    bool RunnableDispatcherThread::isRunning() const
    {
        return this->active;
    }

    void RunnableDispatcherThread::join()
    {
        this->thread.join();
    }

    void RunnableDispatcherThread::run()
    {
        this->runnableDispatcher.runScheduling();
    }

  
    void RunnableDispatcherThread::addRunnable(ScheduledRunnable scheduledRunnable)
    {
        this->runnableDispatcher.addRunnable(scheduledRunnable);
    }

    void RunnableDispatcherThread::addRunnable(
            std::shared_ptr<Runnable> runnable, std::shared_ptr<ScheduleDescription> schedule)
    {
        this->runnableDispatcher.addRunnable(ScheduledRunnable(runnable, schedule));
    }   

    std::thread::id RunnableDispatcherThread::getThreadID()
    {
        return this->thread.get_id();
    }

}

