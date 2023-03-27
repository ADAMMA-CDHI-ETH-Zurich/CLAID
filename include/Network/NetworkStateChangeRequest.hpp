#pragma once
#include "Network/NetworkRequest.hpp"
#include "Reflection/Reflect.hpp"
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