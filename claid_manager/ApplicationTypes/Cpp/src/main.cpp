#include "CLAID.hpp"

int main()
{
    // Manually creating an instance of MyModule. 
    // Later, instantiating and configuring Modules will be done using XML configurations.
    HelloWorld::MyModule* myModule = new HelloWorld::MyModule;

    // Adding a Module to the Runtime.
    CLAID_RUNTIME->addModule(myModule);

    // Starting CLAID Runtime, which automatically will initialize all Modules.
    CLAID_RUNTIME->start();
    return 0;
}
