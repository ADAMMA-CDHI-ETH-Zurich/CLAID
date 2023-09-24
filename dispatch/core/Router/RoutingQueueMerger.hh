#pragma once

#include "absl/strings/str_split.h"
#include "absl/status/status.h"

namespace claid
{
    class RoutingQueueMerger
    {
        public:
            virtual ~RoutingQueueMerger()
            {

            }
            
            virtual absl::Status start() = 0;
            virtual absl::Status stop() = 0;
    };
}