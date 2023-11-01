#pragma once

#include "dispatch/core/shared_queue.hh"
#include "dispatch/proto/claidservice.grpc.pb.h"

using claidservice::DataPackage;


namespace claid {


    class ModuleManager
    {
        private:
            SharedQueue<DataPackage>& fromModuleDispatcherQueue;
            SharedQueue<DataPackage>& toModuleDispatcherQueue;

        public:
            ModuleManager(
                SharedQueue<DataPackage>& fromModuleDispatcherQueue,
                SharedQueue<DataPackage>& toModuleDispatcherQueue) : 
                        fromModuleDispatcherQueue(fromModuleDispatcherQueue), 
                        toModuleDispatcherQueue(toModuleDispatcherQueue)
            {

            }
         
    };
}