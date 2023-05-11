#include "CLAID.hpp"
#include "MyModule.hpp"

namespace HelloWorld
{
    void MyModule::initialize()
    {
        printf("Hello world\n");
    }
}
// This will add the Module to the ClassFactory. 
// It allows instantiating this Module from an XML configuration.
REGISTER_MODULE(HelloWorld::MyModule)