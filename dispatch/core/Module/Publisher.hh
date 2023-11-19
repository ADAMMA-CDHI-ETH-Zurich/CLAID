#pragma once

#include "dispatch/core/shared_queue.hh"
#include "dispatch/proto/claidservice.grpc.pb.h"

#include "dispatch/core/Module/TypeMapping/Mutator.hh"
#include "dispatch/core/Module/TypeMapping/TypeMapping.hh"


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
            std::shared_ptr<DataPackage> package = std::make_shared<DataPackage>();
            package->set_source_module(this->moduleId);
            package->set_channel(this->channelName);

            this->mutator->setPackagePayload(package, data);

            this->toModuleManagerQueue.push_back(package);
        }        
    };

}

