#pragma once
#include "Reflection/RecursiveReflector.hpp"

namespace portaible
{
    template<typename Derived>
    class Deserializer : public RecursiveReflector<Derived>
    { 
        public:
            static constexpr bool isReadOnly = false;

            // Empty function which can be overriden.
            // It is not used by all deserializers, so we do not require it to be there.
            template<typename MemberType, typename Type>
            void pointer_to_member(std::string name, Type MemberType::*ptr)
            {
               
            }

    };
}