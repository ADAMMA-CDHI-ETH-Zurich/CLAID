#include "CLAID.hpp"

namespace HelloWorld
{
    // When creating a Module, inheriting from claid::Module is mandatory.
    class MyModule : public claid::Module
    {
        void initialize();
    };
}
