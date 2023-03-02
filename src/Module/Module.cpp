// For remaining implementations of Module, see Module_impl.hpp
// (was done in separate _impl file, in order to solve cycle dependencies..)

#include "RunTime/RunTime.hpp"

namespace claid
{
    // Somehow compiler does not like the constructor in the Module_imp .. soooo
    Module::Module()
    {
        this->channelManager = &CLAID_RUNTIME->channelManager;
    }
}
// Yes, we do not use the REGISTER_MODULE macro here, because class "Module" should not be loadable by itself (it's just a base class!).
REGISTER_TO_CLASS_FACTORY(claid::Module) 
REGISTER_REFLECTOR_FOR_CLASS(claid::Module, claid::XMLDeserializer, XMLDeserializer) 
REGISTER_REFLECTOR_FOR_CLASS(claid::Module, claid::XMLSerializer, XMLSerializer) 
REGISTER_REFLECTOR_FOR_CLASS(claid::Module, claid::BinarySerializer, BinarySerializer)
REGISTER_REFLECTOR_FOR_CLASS(claid::Module, claid::BinaryDeserializer, BinaryDeserializer)