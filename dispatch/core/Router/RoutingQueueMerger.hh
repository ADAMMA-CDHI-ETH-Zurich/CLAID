#pragma once

#include <tuple>
#include "dispatch/proto/claidservice.pb.h"
#include "dispatch/core/shared_queue.hh"
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
    class RoutingQueueMerger
    {
        private:
            

            RoutingQueueForwarder forwarders[sizeof...(Inputs)];
            const int NumQueues = sizeof...(Inputs);
           

        public:
            RoutingQueueMerger(
                SharedQueue<Output>& outputQueue, 
                SharedQueue<Inputs>&... inputQueues) : forwarders{RoutingQueueForwarder(inputQueues, outputQueue)...}
            {

            }

            absl::Status start()
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
    };
}