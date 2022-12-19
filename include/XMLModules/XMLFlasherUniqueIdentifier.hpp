#include "CLAID.hpp"

namespace claid
{
    struct XMLFlasherUniqueIdentifier
    {
        DECLARE_SERIALIZATION(XMLFlasherUniqueIdentifier);

        uint64_t uniqueIdentifier;
  

        Reflect(XMLFlasherUniqueIdentifier,
            reflectMember(uniqueIdentifier);
        )
    };
}