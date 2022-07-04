#include "ClassFactory/ClassFactory.hpp"
#include "PolymorphicReflector/PolymorphicReflector.hpp"
#include "XML/XMLSerializer.hpp"
#include "XML/XMLDeserializer.hpp"

#define DECLARE_SERIALIZATION(className)\
    DECLARE_CLASS_FACTORY(className)\
    DECLARE_POLYMORPHIC_REFLECTOR(className, portaible::XMLDeserializer)\

#define SERIALIZATION(className)\
    REGISTER_TO_CLASS_FACTORY(className) \
    REGISTER_POLYMORPHIC_REFLECTOR(className, portaible::XMLDeserializer) \
