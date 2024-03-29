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

#include <tuple>
#include "dispatch/proto/claidservice.pb.h"
#include "dispatch/core/shared_queue.hh"
#include "dispatch/core/Router/RoutingQueueMerger.hh"
#include "dispatch/core/Router/RoutingQueueForwarder.hh"

#include <map>
#include <thread>


using claidservice::DataPackage;


namespace claid
{
    // Takes in packages from N input queues and forwards them to a single output queue.
    // Each input queue will be processed by a separate thread, which sleeps until data is available in the queue.
    // The data will then be forwarded to the output queue.
    template<typename Output, typename... Inputs>
    class RoutingQueueMergerGeneric : public RoutingQueueMerger
    {
        private:
            
            RoutingQueueForwarder forwarders[sizeof...(Inputs)];
            const int NumQueues = sizeof...(Inputs);
           

        public:
            RoutingQueueMergerGeneric(
                SharedQueue<Output>& outputQueue, 
                SharedQueue<Inputs>&... inputQueues) : forwarders{RoutingQueueForwarder(inputQueues, outputQueue)...}
            {

            }

            absl::Status start() override final
            {
                for(int i = 0; i < NumQueues; i++)
                {
                    std::cout << "Starting " << i << "\n";
                    absl::Status status = this->forwarders[i].start();
                    if(!status.ok())
                    {
                        return status;
                    }
                }

                return absl::OkStatus();
            }

            absl::Status stop() override final
            {
                for(int i = 0; i < NumQueues; i++)
                {
                    Logger::logInfo("Stopping RoutingQueueForwarder %d", i);
                    absl::Status status = this->forwarders[i].stop();
                    if(!status.ok())
                    {
                        return status;
                    }
                }

                return absl::OkStatus();
            }
    };

    template<typename Output, typename... Inputs>
    inline std::unique_ptr<RoutingQueueMerger> 
        makeUniqueRoutingQueueMerger(SharedQueue<Output>& outputQueue, SharedQueue<Inputs>&... inputQueues)
    {
        auto genericMerger = new RoutingQueueMergerGeneric(outputQueue, inputQueues...);
        std::unique_ptr<typename std::remove_pointer<decltype(genericMerger)>::type> uniqueMerger(genericMerger);
        std::unique_ptr<RoutingQueueMerger> routingQueueMerger = std::move(uniqueMerger);

        return routingQueueMerger;
    }
}