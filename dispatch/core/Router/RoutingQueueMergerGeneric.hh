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