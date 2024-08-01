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

#include "dispatch/core/shared_queue.hh"
#include "dispatch/proto/claidservice.grpc.pb.h"

#include "dispatch/core/Module/TypeMapping/Mutator.hh"
#include "dispatch/core/Module/TypeMapping/TypeMapping.hh"


using claidservice::DataPackage;

namespace claid{

    template<typename T>
    class Publisher
    {
    private:
        const std::string moduleId;
        const std::string channelName;
        SharedQueue<DataPackage>& toModuleManagerQueue;
        Mutator<T> mutator;

    public:
        Publisher(const std::string& moduleId, 
            const std::string& channelName, 
                SharedQueue<DataPackage>& toModuleManagerQueue) : moduleId(moduleId), channelName(channelName), toModuleManagerQueue(toModuleManagerQueue)
        {
            this->mutator = TypeMapping::getMutator<T>();
        }

        void post(const T& data, const Time& timestamp = Time::now())
        {
            std::shared_ptr<DataPackage> package = std::make_shared<DataPackage>();
            package->set_source_module(this->moduleId);
            package->set_channel(this->channelName);
            package->set_unix_timestamp_ms(timestamp.toUnixTimestampMilliseconds());

            this->mutator.setPackagePayload(*package, data);

            this->toModuleManagerQueue.push_back(package);
        }    

        void postToUser(const T& data, const std::string& userId, const Time& timestamp = Time::now())
        {
            std::shared_ptr<DataPackage> package = std::make_shared<DataPackage>();
            package->set_source_module(this->moduleId);
            package->set_channel(this->channelName);
            package->set_unix_timestamp_ms(timestamp.toUnixTimestampMilliseconds());
            package->set_target_user_token(userId);

            this->mutator.setPackagePayload(*package, data);

            this->toModuleManagerQueue.push_back(package);
        }     
    };

}

