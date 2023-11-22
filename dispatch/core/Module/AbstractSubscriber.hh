#pragma once

#include "dispatch/proto/claidservice.grpc.pb.h"

using claidservice::DataPackage;

namespace claid
{
    class AbstractSubscriber
    {
        public:
            virtual ~AbstractSubscriber() {}
            virtual void onNewData(std::shared_ptr<DataPackage> package) = 0;
    };
}