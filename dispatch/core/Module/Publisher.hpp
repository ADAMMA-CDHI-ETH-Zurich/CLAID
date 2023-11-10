#pragma once

#include "dispatch/core/shared_queue.hh"
#include "dispatch/proto/claidservice.grpc.pb.h"

using claidservice::DataPackage;

namespace claid{

    template<typename T>
    class Publisher
    {
    private:
        const std::string moduleId;
        const std::string channelName;
        SharedQueue<DataPackage>& toModuleManagerQueue;
        Mutator<T> mutator;

    public:
        Publisher(const std::string& moduleId, 
            const std::string& channelName, 
                SharedQueue<DataPackage>& toModuleManagerQueue) : moduleId(moduleId), channelName(channelName), toModuleManagerQueue(toModuleManagerQueue)
        {
            this->mutator = TypeMapping::getMutator<T>();
        }

        void post(const T& data)
        {
            DataPackage package;
            package.set_source_host_module(this.moduleId);
            package.set_channel(this.channelName);

            this.mutator->setPackagePayload(package, data);

            this.toModuleManagerQueue.add(dataPackage);
        }        
    };

}
