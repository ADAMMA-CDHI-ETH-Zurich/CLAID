#pragma once
#include "RecursiveReflector.hpp"

namespace portaible
{
    template<typename Derived>
    class Deserializer : public RecursiveReflector<Derived>
    { 
        public:
            static std::false_type isReadOnly;


    };
}