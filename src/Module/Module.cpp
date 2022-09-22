// For remaining implementations of Module, see Module_impl.hpp
// (was done in separate _impl file, in order to solve cycle dependencies..)

#include "RunTime/RunTime.hpp"

namespace portaible
{
    // Somehow compiler does not like the constructor in the Module_imp .. soooo
    Module::Module()
    {
        this->channelManager = &PORTAIBLE_RUNTIME->channelManager;
    }
}

REGISTER_MODULE(portaible::Module)