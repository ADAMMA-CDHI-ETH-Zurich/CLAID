#pragma once 

#include <functional>

#include "dispatch/proto/claidservice.grpc.pb.h"

using claidservice::DataPackage;

namespace claid {

template<typename T>
class Mutator
{
    // In Java, Protobuf types are immutable. 
    // Hence, the setter has to return a new DataPacke.

    std::function<void(DataPackage&, const T&)> setter;
    std::function<T(const DataPackage& )> getter;
    
public:
    Mutator(std::function<void(DataPackage&, const T&)> setter, 
        std::function<T(const DataPackage&)> getter) : setter(setter), getter(getter)
    {

    }

    void setPackagePayload(DataPackage& packet, const T& value) 
    {
        setter(packet, value);
    }

    T getPackagePayload(const DataPackage& packet) 
    {
        return getter(packet);
    }
};


}

