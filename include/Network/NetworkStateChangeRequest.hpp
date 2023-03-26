#pragma once
#include "Network/NetworkRequest.hpp"
namespace claid
{
    class NetworkStateChangeRequest
    {
        public:
            NetworkRequest networkRequest;

            Reflect(NetworkStateChangeRequest,
                reflectMember(networkRequest);
            )
    };
}