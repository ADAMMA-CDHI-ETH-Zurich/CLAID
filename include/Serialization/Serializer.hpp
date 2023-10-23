#pragma once

#include "Reflection/RecursiveReflector.hpp"
#include "Serialization/AbstractSerializer.hpp"
namespace claid
{

    
    template<typename Derived>
    class Serializer : public AbstractSerializer, public RecursiveReflector<Derived>
    { 
        public:
            static constexpr bool isReadOnly = true;

            // Empty function which can be overriden.
            // It is not used by all Serializers, so we do not require it to be there.
            template<typename MemberType, typename Type>
            void pointer_to_member(std::string name, Type MemberType::*ptr)
            {
               
            }
            
            template<typename Class, typename ReturnType, typename... Args>
            void pointer_to_function(std::string name, ReturnType (Class::*function)(Args...))
            {
                
            }

            template<typename ReturnType, typename ClassType, typename... Args>
            void pointer_to_function(std::string name, ReturnType (ClassType::*ptr)(Args...) const)
            {

            }


    };
}