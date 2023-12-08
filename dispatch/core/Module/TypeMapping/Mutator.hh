#pragma once 

#include <functional>
#include "dispatch/core/Logger/Logger.hh"

#include "dispatch/proto/claidservice.grpc.pb.h"

using claidservice::DataPackage;

namespace claid {

template<typename T>
class Mutator
{
    // In Java, Protobuf types are immutable. 
    // Hence, the setter has to return a new DataPacke.

    std::function<void(DataPackage&, const T&)> setter;
    std::function<void (const DataPackage&, T& )> getter;
    
public:
    Mutator()
    {
        
    }

    Mutator(std::function<void(DataPackage&, const T&)> setter, 
        std::function<void (const DataPackage&, T& )> getter) : setter(setter), getter(getter)
    {

    }

    void setPackagePayload(DataPackage& packet, const T& value) 
    {
        setter(packet, value);
    }

    void getPackagePayload(const DataPackage& packet, T& returnValue) 
    {
        getter(packet, returnValue);
    }
};


}

