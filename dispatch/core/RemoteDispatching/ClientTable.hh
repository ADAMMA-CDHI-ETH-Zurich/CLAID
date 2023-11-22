#pragma once

#include "dispatch/proto/claidservice.grpc.pb.h"
#include "dispatch/core/shared_queue.hh"

using claidservice::DataPackage;

namespace claid {

    // Analog to the ModuleTable for the LocalRouter and the HostUserTable for the ServerRouter,
    // the ClientTable enables the ClientRouter to access the queue to route packages via a connection to an external server.
    
    class ClientTable
    {
        private:
            SharedQueue<DataPackage> toRemoteClientQueue;

        public:
            ClientTable();

            SharedQueue<DataPackage>& getQueue();

    };
}