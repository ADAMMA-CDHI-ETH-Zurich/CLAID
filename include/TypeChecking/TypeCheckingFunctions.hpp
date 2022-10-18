#pragma once
#include "CompileTimeTypeNameDemangling.hpp"
#include "TypeChecking/Invokers/TypeNameInvoker.hpp"
namespace claid
{
    namespace TypeChecking
    {
        // Use extern to make sure the adress of this function is always the same no matter where it is used within the code.
        template<typename T>
        extern intptr_t getDataTypeUniqueIdentifier()
        {
            // Use the address of this function for the current data type as unique identifier.
			// Windows is a bitch. 
			// On Linux/Unix, can just do return reinterpret_cast<intptr_t>(&getDataTypeUniqueIdentifier<T>)
			auto fptr = &getDataTypeUniqueIdentifier<T>;
			void* tmp = reinterpret_cast<void *&>(fptr);
            return reinterpret_cast<intptr_t>(tmp);
        }

    
        template<typename T>
        std::string getCompilerSpecificRunTimeNameOfObject(T& t)
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
        // template<>
        // std::string getDataTypeRTTIString<std::string>(std::string& t)
        // {
        //     return "std::string";
        // }




        // Why use this instead of getCompilerIndependentTypeNameOfClass()?
        // Because this function supports ALL types, while getCompilerIndependentTypeNameOfClass() is only
        // enabled for certain types.
        template<typename T>
        static std::string getCompilerSpecificCompileTypeNameOfClass()
        {
            #ifdef __PORTAIBLE_USE_TYPEID_FOR_COMPILE_TIME_TYPE_NAME__
                return compileTimeTypeNameByTypeid<T>();
            #else
                return compileTimeTypeNameByUsingFunctionName<T>().toStdString();
            #endif
        }

        template<typename T>
        static std::string getCompilerIndependentTypeNameOfClass()
        {
            return TypeNameInvoker<T>::call();
        }
    }
}

        