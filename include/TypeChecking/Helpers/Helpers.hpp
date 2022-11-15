#pragma once

namespace claid
{
    namespace TypeChecking
    {
        namespace Helpers
        {
            template< typename T >
            struct getBitsetSize;

            template< size_t Len > 
            struct getBitsetSize< std::bitset< Len > >
            {
                enum { Length = Len };
            };  


            template<typename T>
            struct getDereferencedType;

            template<typename T>
            struct getDereferencedType<T*>
            {
                typedef T type;
            };
        }
    }
}