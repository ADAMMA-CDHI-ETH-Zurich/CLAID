#include "dispatch/core/Module/ModuleRef.hh"
#include "dispatch/core/Module/Module.hh"

namespace claid
{

    void ModuleRef::moduleError(const std::string& error) const
    {
        this->module->moduleError(error);
    }

    void ModuleRef::moduleWarning(const std::string& warning) const
    {
        this->module->moduleWarning(warning);
    }

    std::string ModuleRef::getId() const
    {
        return this->module->getId();
    }


}