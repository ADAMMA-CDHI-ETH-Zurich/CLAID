#pragma once

#include "dispatch/proto/claidservice.grpc.pb.h"

using claidservice::DataPackage;

namespace claid
{
    class AbstractSubscriber
    {
        public:
            virtual void onNewData(DataPackage data) = 0;
    };
}