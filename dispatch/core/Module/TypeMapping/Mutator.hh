/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Stephan Altmüller
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*         http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
***************************************************************************/

#pragma once 

#include <functional>
#include "dispatch/core/Logger/Logger.hh"

#include "dispatch/proto/claidservice.pb.h"
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/any.pb.h>
using claidservice::DataPackage;

namespace claid {

template<typename T>
class Mutator
{
    std::shared_ptr<const google::protobuf::Message> mappedProtoType;

    std::function<void(DataPackage&, const T&)> setter;
    std::function<void (const DataPackage&, T& )> getter;
    
public:
    Mutator()
    {
        
    }

    Mutator(std::shared_ptr<const google::protobuf::Message> mappedProtoType,
        std::function<void(DataPackage&, const T&)> setter, 
        std::function<void (const DataPackage&, T& )> getter) : mappedProtoType(mappedProtoType), setter(setter), getter(getter)
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

    std::string getMessageTypeName()
    {
        if(this->mappedProtoType != nullptr)
        {
            return this->mappedProtoType->GetDescriptor()->full_name();
        }

        return "Unknown";
    }
};


}

