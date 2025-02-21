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

#include "dispatch/proto/claidservice.pb.h"
#include "dispatch/core/Module/TypeMapping/TypeMapping.hh"




using claidservice::DataPackage;
using claidservice::CtrlType;
using claidservice::ModuleAnnotation;
using claidservice::PropertyHint;
using claidservice::PropertyType;

namespace claid
{
    class ModuleAnnotator
    {
        private:
            ModuleAnnotation annotation;

            std::string moduleType;

            template <typename T>
            DataPackage prepareExamplePackage(const std::string& moduleId, const std::string& channelName, bool isPublisher) 
            {
                DataPackage dataPackage;

                if(isPublisher)
                {
                    // Only set module Id, host will be added by Middleware later.
                    dataPackage.set_source_module(moduleId);//concatenateHostModuleAddress(this->host, moduleId));
                }
                else
                {
                    // Only set module Id, host will be added by Middleware later.
                    dataPackage.set_target_module(moduleId);//concatenateHostModuleAddress(this->host, moduleId));
                }
                dataPackage.set_channel(channelName);

                Mutator<T> mutator = TypeMapping::getMutator<T>();
                T exampleInstance = T();
                mutator.setPackagePayload(dataPackage, exampleInstance);

                return dataPackage;   
            }


        public:



            ModuleAnnotator(const std::string& moduleType);

            void setModuleDescription(const std::string& moduleDescription);
            void setModuleCategory(const std::string& moduleCategory);

            PropertyHint makeDefaultProperty();
            PropertyHint makeEnumProperty(const std::vector<std::string>& enumValues);
            PropertyHint makeIntegerProperty(int64_t min, int64_t max);
            PropertyHint makePathProperty();

            void describeProperty(const std::string& propertyName, const std::string& propertyDescription);
            void describeProperty(const std::string& propertyName, const std::string& propertyDescription, PropertyHint propertyHint);

            template<typename T>
            void describePublishChannel(const std::string& channelName, const std::string& channelDescription)
            {
                DataPackage examplePackage = prepareExamplePackage<T>(this->moduleType, channelName, true);
                this->annotation.add_channel_definition()->CopyFrom(examplePackage);
                this->annotation.add_channel_description(channelDescription);
            }

            template<typename T>
            void describeSubscribeChannel(const std::string& channelName, const std::string& channelDescription)
            {
                DataPackage examplePackage = prepareExamplePackage<T>(this->moduleType, channelName, false);
                this->annotation.add_channel_definition()->CopyFrom(examplePackage);
                this->annotation.add_channel_description(channelDescription);
            }

            const ModuleAnnotation& getAnnotation() const;

            
    };
}