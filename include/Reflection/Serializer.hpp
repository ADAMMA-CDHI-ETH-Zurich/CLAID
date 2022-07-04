#pragma once

#include "RecursiveReflector.hpp"

namespace portaible
{
    template<typename Derived>
    class Serializer : public RecursiveReflector<Derived>
    { 
        public:
            static std::true_type isReadOnly;

            
    };
}