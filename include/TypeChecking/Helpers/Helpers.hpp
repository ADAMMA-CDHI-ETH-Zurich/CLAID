#pragma once

namespace portaible
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
        }
    }
}