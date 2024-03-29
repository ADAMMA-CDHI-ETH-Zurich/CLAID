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

#include "dispatch/core/Module/ModuleAnnotator/ModuleAnnotator.hh"

namespace claid {
    
    ModuleAnnotator::ModuleAnnotator(const std::string& moduleType) : moduleType(moduleType)
    {

    }

    void ModuleAnnotator::setModuleDescription(const std::string& moduleDescription)
    {
        this->annotation.set_module_description(moduleDescription);
    }

    void ModuleAnnotator::setModuleCategory(const std::string& moduleCategory)
    {
        this->annotation.set_module_category(moduleCategory);
    }

    PropertyHint ModuleAnnotator::makeDefaultProperty()
    {
        PropertyHint propertyHint;
        propertyHint.set_property_type(PropertyType::PROPERTY_TYPE_DEFAULT);
        return propertyHint;
    }

    PropertyHint ModuleAnnotator::makeEnumProperty(const std::vector<std::string>& enumValues)
    {
        PropertyHint propertyHint;
        propertyHint.set_property_type(PropertyType::PROPERTY_TYPE_ENUM);

        for(const std::string& value : enumValues)
        {
            *propertyHint.add_property_type_enum_values() = value;
        }   
        return propertyHint;
    }

    PropertyHint ModuleAnnotator::makeIntegerProperty(int64_t min, int64_t max)
    {
        PropertyHint propertyHint;
        propertyHint.set_property_type(PropertyType::PROPERTY_TYPE_INT);

        propertyHint.set_property_type_int_min(min);
        propertyHint.set_property_type_int_max(max);
        return propertyHint;
    }

    PropertyHint ModuleAnnotator::makePathProperty()
    {
        PropertyHint propertyHint;
        propertyHint.set_property_type(PropertyType::PROPERTY_TYPE_PATH);

        return propertyHint;
    }

    void ModuleAnnotator::describeProperty(const std::string& propertyName, const std::string& propertyDescription)
    {
        this->describeProperty(propertyName, propertyDescription, makeDefaultProperty());
    }

    void ModuleAnnotator::describeProperty(const std::string& propertyName, const std::string& propertyDescription, PropertyHint propertyHint)
    {
        this->annotation.add_properties(propertyName);
        this->annotation.add_property_descriptions(propertyDescription);
        *this->annotation.add_property_hints() = propertyHint;
    }

    const ModuleAnnotation& ModuleAnnotator::getAnnotation() const
    {
        return this->annotation;
    }

}