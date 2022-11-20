#include "CLAID.hpp"

namespace claid
{
    struct XMLConfigFlashedModulesDescription
    {
        DECLARE_SERIALIZATION(XMLConfigFlashedModulesDescription);

        uint64_t uniqueIdentifier;
        std::vector<std::string> flashedModuleNames;

        Reflect(XMLConfigFlashedModulesDescription,
            reflectMember(uniqueIdentifier);
            reflectMember(flashedModuleNames);
        )
    };
}