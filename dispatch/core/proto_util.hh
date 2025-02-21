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

#ifndef PROTO_UTIL_HH_
#define PROTO_UTIL_HH_

#include <string>

#include <google/protobuf/text_format.h>
#include "dispatch/proto/claidservice.pb.h"

using claidservice::DataPackage;
namespace claid {
  // Returns a string representation of the protobuf messsage.
  std::string messageToString(const google::protobuf::Message& msg);

  std::string dataPackagePayloadCaseToString(const DataPackage& dataPackage);
  std::string dataPackageDataTypeName(const DataPackage& dataPackage);

}  // namespace claid

#endif  // PROTO_UTIL_HH_
