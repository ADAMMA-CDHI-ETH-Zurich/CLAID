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
                    package = this->inputQueue.pop_front();
                    this->outputQueue.push_back(package);
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
                this->thread = make_unique<std::thread>([this]() { forwardPackages(); });

                return absl::OkStatus();
            }
    };
}