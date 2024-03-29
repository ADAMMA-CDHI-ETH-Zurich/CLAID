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


#include <map>
#include <string>
#include <vector>

#include "dispatch/core/Configuration/UniqueKeyMap.hh"

namespace claid
{
    struct ModuleDescription
    {
        std::string id;
        std::string moduleClass;
        std::string host;
        std::map<std::string, std::string> inputChannels;
        std::map<std::string, std::string> outputChannels;
        google::protobuf::Struct properties;

        ModuleDescription()
        {

        }

        ModuleDescription(const std::string& id, 
            const std::string& moduleClass, 
            const std::string& host, 
            const google::protobuf::Struct properties) : id(id), moduleClass(moduleClass), host(host), properties(properties)
        {

        }
    };

    typedef UniqueKeyMap<ModuleDescription> ModuleDescriptionMap;
}