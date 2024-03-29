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

#include "dispatch/proto/claidservice.pb.h"
#include "dispatch/core/shared_queue.hh"

#include <map>
#include <thread>


using claidservice::DataPackage;


namespace claid
{
    // Spawns a new thread and waits for new data to arrive on an input queue.
    // This data will then be forwarded to the output queue.
    class RoutingQueueForwarder
    {
        private:
            SharedQueue<DataPackage>& inputQueue;
            SharedQueue<DataPackage>& outputQueue;

            std::unique_ptr<std::thread> thread;
            bool active = false;

            void forwardPackages()
            {
                while(this->active)
                {
                    std::shared_ptr<DataPackage> package;
                        Logger::logInfo("RoutingQueueForwarder interruptable pop front %lu %lu", this, &inputQueue);
                    package = this->inputQueue.interruptable_pop_front();

                    if(!this->outputQueue.is_closed() && package != nullptr)
                    {
                        this->outputQueue.push_back(package);
                    }   
                }
            }

        public:

            RoutingQueueForwarder(
                SharedQueue<DataPackage>& inputQueue, 
                SharedQueue<DataPackage>& outputQueue) : inputQueue(inputQueue), outputQueue(outputQueue)
            {

            }

            absl::Status start()
            {
                std::cout << "Queue IDs " << reinterpret_cast<intptr_t>(&inputQueue) << " " << reinterpret_cast<intptr_t>(&outputQueue) << "\n";
                if(thread.get() != nullptr || this->active)
                {
                    return absl::AlreadyExistsError("RoutingQueueForwarder: Start was called twice.");
                }
                this->active = true;
                this->thread = std::make_unique<std::thread>([this]() { forwardPackages(); });

                return absl::OkStatus();
            }

            absl::Status stop()
            {
                if(thread.get() == nullptr || !this->active)
                {
                    return absl::InvalidArgumentError("RoutingQueueForwarder: Stop failed, forwarder was not started before.");
                }
                Logger::logInfo("Stopping RoutingQueueForwarder %lu %lu", this, &inputQueue);
                this->active = false;
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                this->inputQueue.interruptOnce();
                this->thread->join();
                this->thread = nullptr;

                return absl::OkStatus();
            }
    };
}