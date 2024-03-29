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

#include "dispatch/core/proto_util.hh"

#include <google/protobuf/text_format.h>

std::string claid::messageToString(const google::protobuf::Message& msg) {
    std::string buf;
    if (google::protobuf::TextFormat::PrintToString(msg, &buf)) {
        return buf;
    }
    return "Message not valid (partial content: " +
            msg.ShortDebugString();
}

std::string claid::dataPackagePayloadCaseToString(const DataPackage& dataPackage)
{
    const google::protobuf::Descriptor* descriptor = dataPackage.GetDescriptor();
    const google::protobuf::Reflection* reflection = dataPackage.GetReflection();
    const google::protobuf::OneofDescriptor* oneofDescriptor = descriptor->FindOneofByName("payload_oneof");

    for (int i = 0; i < oneofDescriptor->field_count(); i++) {
        const google::protobuf::FieldDescriptor* fieldDescriptor = oneofDescriptor->field(i);
        if (reflection->HasField(dataPackage, fieldDescriptor)) {
            const std::string& fieldName = fieldDescriptor->name();
            return fieldName;
        }
    }
    return "";
}
 