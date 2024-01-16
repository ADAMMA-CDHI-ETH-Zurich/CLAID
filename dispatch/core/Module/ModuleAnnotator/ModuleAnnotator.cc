#include "dispatch/core/Module/ModuleAnnotator/ModuleAnnotator.hh"

namespace claid {
    
    ModuleAnnotator::ModuleAnnotator(const std::string& moduleType) : moduleType(moduleType)
    {

    }

    void ModuleAnnotator::setModuleDescription(const std::string& moduleDescription)
    {
        this->annotation.set_module_description(moduleDescription);
    }

    void ModuleAnnotator::describeProperty(const std::string& propertyName, const std::string& propertyDescription)
    {
        this->annotation.add_properties(propertyName);
        this->annotation.add_property_descriptions(propertyDescription);
    }

    const ModuleAnnotation& ModuleAnnotator::getAnnotation() const
    {
        return this->annotation;
    }

}