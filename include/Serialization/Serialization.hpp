#pragma once
#include "ClassFactory/ClassFactory.hpp"
#include "PolymorphicReflector/PolymorphicReflector.hpp"
#include "XML/XMLSerializer.hpp"
#include "XML/XMLDeserializer.hpp"
#include "Binary/BinarySerializer.hpp"
#include "Binary/BinaryDeserializer.hpp"

#define DECLARE_SERIALIZATION(className) \
    DECLARE_CLASS_FACTORY(className)\
    DECLARE_POLYMORPHIC_REFLECTOR(className, portaible::XMLSerializer, XMLSerializer)\
    DECLARE_POLYMORPHIC_REFLECTOR(className, portaible::XMLDeserializer, XMLDeserializer)\
    DECLARE_POLYMORPHIC_REFLECTOR(className, portaible::BinarySerializer, BinarySerializer)\
    DECLARE_POLYMORPHIC_REFLECTOR(className, portaible::BinaryDeserializer, BinaryDeserializer)

#define REGISTER_SERIALIZATION(className)\
    REGISTER_TO_CLASS_FACTORY(className) \
    REGISTER_POLYMORPHIC_REFLECTOR(className, portaible::XMLDeserializer, XMLDeserializer) \
    REGISTER_POLYMORPHIC_REFLECTOR(className, portaible::XMLSerializer, XMLSerializer) \
    REGISTER_POLYMORPHIC_REFLECTOR(className, portaible::BinarySerializer, BinarySerializer)\
    REGISTER_POLYMORPHIC_REFLECTOR(className, portaible::BinaryDeserializer, BinaryDeserializer)

#define REFLECT_BASE(Reflector, Base) Base::reflect(r);