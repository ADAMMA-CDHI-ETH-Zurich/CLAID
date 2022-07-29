#pragma once
#include "BinaryData.hpp"
#include "BinaryDataReader.hpp"
#include "Reflection/Serializer.hpp"
#include "TypeChecking/TypeCheckingFunctions.hpp"

namespace portaible
{
    class BinaryDeserializer : public Deserializer<BinaryDeserializer>
    {
    
        private:
            BinaryDataReader binaryDataReader;

        public:
            

            BinaryDeserializer()
            {
                
            }

           

            template<typename T>
            void callFloatOrDouble(const char* property, T& member)
            {
                this->binaryDataReader.read(member);
            }   

            // Also includes any variants of signed, unsigned, short, long, long long, ...
            template<typename T>
            void callInt(const char* property, T& member)
            {
                this->binaryDataReader.read(member);
            }

            void callBool(const char* property, bool& member)
            {
                this->binaryDataReader.read(member);
            }

            void callChar(const char* property, char& member)
            {
                this->binaryDataReader.read(member);
            }

            void callString(const char* property, std::string& member)
            {
                this->binaryDataReader.readString(member);
            }

            template<typename T>
            void callBeginClass(const char* property, T& member)
            {
                
            }

            template<typename T>
            void callEndClass(const char* property, T& member)
            {

            }


            template<typename T>
            void callPointer(const char* property, T*& member)
            {
                std::string className;
                this->binaryDataReader.readString(className);

                // Check if ClassFactory is registered for className.
                if (!ClassFactory::ClassFactory::getInstance()->isFactoryRegisteredForClass(className))
                {
                    PORTAIBLE_THROW(portaible::Exception, "BinaryDeserializer failed to deserialize object from binary data. Class \"" << className << "\" was not registered to ClassFactory and is unknown.");
                } 

                member = ClassFactory::ClassFactory::getInstance()->getNewInstanceAndCast<T>(className);

                PolymorphicReflector::WrappedReflectorBase<BinaryDeserializer>* polymorphicReflector;
                if (!PolymorphicReflector::PolymorphicReflector<BinaryDeserializer>::getInstance()->getReflector(className, polymorphicReflector))
                {
                    PORTAIBLE_THROW(portaible::Exception, "BinaryDeserializer failed to deserialize object from binary. Member \"" << property << "\" is a pointer type with it's class specified as \"" << className << "\". However, no PolymorphicReflector was registered for class \"" << className << "\". Was PORTAIBLE_SERIALIZATION implemented for this type?");
                }

                polymorphicReflector->invoke(*this, static_cast<void*>(member));               
            }
            
            template<typename T>
            void callSharedPointer(const char* property, T& member)
            {
                typedef typename T::element_type BaseTypeT;
                BaseTypeT* pointer;
                this->callPointer<BaseTypeT>(property, pointer);
                member = T(pointer, std::default_delete<BaseTypeT>());
            }

            void count(const std::string& name, size_t& count)
            {
                // TODO: IMPLEMENT IN ORDER TO SUPPORT VECTOR, MAP ETC.
                this->binaryDataReader.read(count);
            }

            void countElements(size_t& count)
            {
                this->binaryDataReader.read(count);
            }

            void beginSequence()
            {
                // Do nothing
            }

            void endSequence()
            {
                // Do nothing
            }

            void read(char*& data, size_t size)
            {
                this->binaryDataReader.readBytes(data, size);
            }

            

            template<typename T> 
            void deserialize(T& obj, BinaryDataReader& binaryDataReader)
            {
                this->binaryDataReader = binaryDataReader;

                // Read data type string and check if it matches the data type of obj.
                std::string name = portaible::getDataTypeRTTIString<T>();
                std::string storedName;


                this->binaryDataReader.readString(storedName);

                if(name != storedName)
                {
                    PORTAIBLE_THROW(Exception, "Error, failed to deserialize object from binary data because of mismatching types."
                                                "The data type of the object is " << name << " but the serialized data is of type " << storedName);
                }

                invokeReflectOnObject(obj);
            }

            void enforceName(std::string& name, int idInSequence = 0)
            {
                // Some serializers, like BinarySerializer, might not store the members name (i.e., property parameters),
                // as it is not necessary to be known in the binary data).
                // For some cases, however, it might be necessary to store such strings in the serialized data nevertheless,
                // as it might be needed for deserialization etc.
                // Thus, this function allows to make sure the string "name" is explicitly stored.
                
                this->binaryDataReader.readString(name);
            }

    };
}