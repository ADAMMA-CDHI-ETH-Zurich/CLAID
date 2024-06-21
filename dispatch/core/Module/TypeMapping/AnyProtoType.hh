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

#include "google/protobuf/message.h"
#include "dispatch/proto/claidservice.pb.h"

namespace claid {

class AnyProtoType
{
    private:
        std::shared_ptr<google::protobuf::Message> message;

    public:
        AnyProtoType()
        {
            // If a Module subscribes/publihses a Channel using AnyProtoType, 
            // then during the initialization of the Module, an example DataPackage will be created,
            // which is used to tell the Middleware what data is expected by the Module.
            // Hence, during publish/subscribe, a Mutator will be created for AnyProtoType, which will be used
            // to set an example instance for a DataPackage.
            // For this to work, message cannot be null.
            // Hence, in the case of the default constructor, we have to initialize message with a valid proto type.
            // The type itself does not matter, because all proto types are represented by DataPackage::PayloadOneOfCase::kBlobVal.
            // Therefore, we choose NumberArray here, because it is a small data type with only minimal overhead.
            message = std::static_pointer_cast<google::protobuf::Message>(std::make_shared<claidservice::CLAIDANY>());
        }

        AnyProtoType(std::shared_ptr<google::protobuf::Message> message) : message(message)
        {

        }

        template<typename T>
        AnyProtoType(std::shared_ptr<T> message)
        {
            this->message = std::static_pointer_cast<google::protobuf::Message>(message);
        }

        void setMessage(std::shared_ptr<google::protobuf::Message> message)
        {
            this->message = message;
        }

        std::shared_ptr<const google::protobuf::Message> getMessage() const
        {
            return this->message;
        }
};

}