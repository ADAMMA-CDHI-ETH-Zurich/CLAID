#pragma once
#include <string>

namespace claid
{
    class Module;
    // Helper class to avoid circular dependencies.
    class ModuleRef
    {
        private:
            Module* module;
        
        public:
            ModuleRef(Module* module) : module(module)
            {

            }


            void moduleError(const std::string& error) const;
            void moduleWarning(const std::string& warning) const;
            std::string getId() const;
    };
}