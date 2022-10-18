#pragma once
#include "static_string.hpp"


#ifdef __GNUG__

    // When using GNU C++ compiler, we can demangle the RTTI typeid::name string
    // and provide a more readable identifier for data types.
    #include <cstdlib>
    #include <memory>
    #include <cxxabi.h>

#endif



namespace claid
{


    namespace TypeChecking
    {

        static void remove_prefix(std::string& full_string, std::string prefix)
        {
            full_string = full_string.substr(prefix.length());
        }
        
        static void remove_suffix(std::string& full_string, std::string suffix)
        {
            full_string = full_string.substr(0, full_string.length() - suffix.length());
        }


        template<typename T>
        static auto compileTimeTypeNameByTypeid()
        {
            std::string name = typeid(T).name();


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


        // Taken from https://stackoverflow.com/questions/81870/is-it-possible-to-print-a-variables-type-in-standard-c/56766138#56766138
        // Get's the type name of a type without using typeid().
        // Benefits: 
        // 1st Evaluated at compile time (typeid(T).name() itself is too, however when using abi::__cxa_demangle, then the whole
        // function to retrieve a demangled type name (i.e. compileTimeTypeNameByTypeID, see above)) is evaluated at runtime).
        // Therefore, it allows to get a type name and evaluate if it is contained in a certain namespace (see "isInNamespace" of static_string),
        // which would not be possible when using typeid(T).name() together with abi::__cxa_demangle;

        // 2nd Automatic demangling / automatically using the compilers demangling method to get the type name,
        // as __PRETTY_FUNCTION__ and __FUNCSIG__ should provide human readable strings (usually they might be used for debugging).
        // I.e.: no need to use abi::__cxa_demangle or other specific demangling methods.
        template <typename T>
        constexpr auto compileTimeTypeNameByUsingFunctionName() 
        {
            #ifdef __clang__
                static_string p = __PRETTY_FUNCTION__;

                // __PRETTY_FUNCTION__ is going to be sth like
                // auto claid::TypeChecking::compileTimeTypeNameByUsingFunctionName() [T = std::string]
                // What we are interested in is only std::string, thus we remove the prefix and suffix.

                const int totalLength = constexpr_length_of_c_string(__PRETTY_FUNCTION__);
                const int prefixLength = constexpr_length_of_c_string("auto claid::TypeChecking::compileTimeTypeNameByUsingFunctionName() [T = "); 
                const int suffixLength = constexpr_length_of_c_string("]");
            #elif defined(__GNUC__)
                static_string p = __PRETTY_FUNCTION__;
                const int totalLength = constexpr_length_of_c_string(__PRETTY_FUNCTION__);
                const int prefixLength = constexpr_length_of_c_string("auto claid::TypeChecking::compileTimeTypeNameByUsingFunctionName() [T = "); 
                const int suffixLength = constexpr_length_of_c_string("]");
                
            #elif defined(_MSC_VER)
                static_string p = __FUNCSIG__;

                const int totalLength = constexpr_length_of_c_string(__FUNCSIG__);
                const int prefixLength = constexpr_length_of_c_string("auto __cdecl claid::TypeChecking::compileTimeTypeNameByUsingFunctionName<"); 
                const int suffixLength = constexpr_length_of_c_string(">(void)");
            #else
                static_assert(false, "Unsupported compiler in function compileTimeTypeNameByUsingFunctionName. Retrieving compile time name of a class by using a function name (i.e., automatic demangling),"
                "Has only been implemented for clang, gcc and MSCV.\n"
                "However, it should be easy to implement this for any other compiler, as long as it provides a method to get the name of the function that currently is compiled.\n"
                "If so, implementing support for this compiler only requires to write 3 simple lines of code! So do not give up just yet and have a look at the file CompileTimeTypeNameDemangling.hpp");
            #endif

           
            static_assert(prefixLength + suffixLength < totalLength, "Error in function compileTimeTypeNameByUsingFunctionName. Cannot get substring of functio name, as length of prefix + length of suffix is greater than length of function name itself." 
            "Very likely, an outdated or unsupported compiler is used." );
            
            return static_string(p.data() + prefixLength, p.size() - prefixLength - suffixLength);
        }

    }
}