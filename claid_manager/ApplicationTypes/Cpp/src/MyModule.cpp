#include "CLAID.hpp"

namespace HelloWorld
{
    // When creating a Module, inheriting from claid::Module is mandatory.
    class MyModule : public claid::Module
    {
        void initialize()
        {
            printf("Hello world\n");
        }
    };
}
// This will add the Module to the ClassFactory. 
// It allows instantiating this Module from an XML configuration.
REGISTER_MODULE(HelloWorld::MyModule)