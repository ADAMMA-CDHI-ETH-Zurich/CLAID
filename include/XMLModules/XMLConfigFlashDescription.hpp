#include "CLAID.hpp"

namespace claid
{
    struct XMLConfigFlashDescription
    {

        uint64_t uniqueIdentifier;
        std::string xmlConfigData;

        Reflect(XMLConfigFlashDescription,
            reflectMember(uniqueIdentifier);
            reflectMember(xmlConfigData);
        )
    };
}