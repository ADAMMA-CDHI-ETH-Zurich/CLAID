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
// Yes, we do not use the REGISTER_MODULE macro here, because class "Module" should not be loadable by itself (it's just a base class!).
REGISTER_TO_CLASS_FACTORY(portaible::Module) 
REGISTER_POLYMORPHIC_REFLECTOR(portaible::Module, portaible::XMLDeserializer, XMLDeserializer) 
REGISTER_POLYMORPHIC_REFLECTOR(portaible::Module, portaible::XMLSerializer, XMLSerializer) 
REGISTER_POLYMORPHIC_REFLECTOR(portaible::Module, portaible::BinarySerializer, BinarySerializer)
REGISTER_POLYMORPHIC_REFLECTOR(portaible::Module, portaible::BinaryDeserializer, BinaryDeserializer)