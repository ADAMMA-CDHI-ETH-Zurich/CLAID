#pragma once

#include "Reflection/RecursiveReflector.hpp"

namespace portaible
{
    template<typename Derived>
    class Serializer : public RecursiveReflector<Derived>
    { 
        public:
            static constexpr bool isReadOnly = true;

            // Empty function which can be overriden.
            // It is not used by all Serializers, so we do not require it to be there.
            template<typename MemberType, typename Type>
            void pointer_to_member(std::string name, Type MemberType::*ptr)
            {
               
            }
            
    };
}