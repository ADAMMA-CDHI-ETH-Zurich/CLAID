#include "CLAID.hpp"

namespace claid
{
    struct XMLFlasherUniqueIdentifier
    {

        uint64_t uniqueIdentifier;
  

        Reflect(XMLFlasherUniqueIdentifier,
            reflectMember(uniqueIdentifier);
        )
    };
}