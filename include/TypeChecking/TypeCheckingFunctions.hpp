#pragma once
#ifdef __GNUG__

// When using GNU C++ compiler, we can demangle the RTTI typeid::name string
// and provide a more readable identifier for data types.
#include <cstdlib>
#include <memory>
#include <cxxabi.h>

#endif


namespace portaible
{
    template<typename T>
    intptr_t getDataTypeUniqueIdentifier()
    {
        // Use the address of this function for the current data type as unique identifier.
        return reinterpret_cast<intptr_t>(&getDataTypeUniqueIdentifier<T>);
    }

    template<typename T>
    std::string getDataTypeRTTIString(T& t)
    {
        std::string name = typeid(t).name();


        #ifdef __GNUG__
            // When using GNU C++ compiler, we demangle the string returned from typeid::name
            int status = -4; // some arbitrary value to eliminate the compiler warning

            struct handle {
                char* p;
                handle(char* ptr) : p(ptr) { }
                ~handle() { std::free(p); }
            };

            handle result( abi::__cxa_demangle(name.c_str(), NULL, NULL, &status) );

            return (status==0) ? result.p : name ;
        #else
            // Otherwise we return the mangled string.
            
            if (name.find("struct") != std::string::npos)
            {
                name = name.substr(strlen("struct "), name.size() - strlen("struct "));
            }
            return name;
        #endif

    }

    template<typename T>
    std::string getDataTypeRTTIString()
    {
        // Sadly, we need an instance in order to use typeid :(
        // So this generates some overhead :/
        // Don't use it that often!    
        T t;

        return getDataTypeRTTIString(t);
    }
}

        