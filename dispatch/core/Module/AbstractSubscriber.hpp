#pragma once

#include "dispatch/proto/claidservice.grpc.pb.h"

using claidservice::DataPackage;

namespace claid
{
    class AbstractSubscriber
    {
        public:
            void onNewData(DataPackage data);
    };
}