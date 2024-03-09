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