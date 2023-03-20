#pragma once
#include "BinaryData.hpp"
#include "BinaryDataReader.hpp"
#include "Serialization/Serializer.hpp"
#include "Serialization/Deserializer.hpp"
#include "TypeChecking/TypeCheckingFunctions.hpp"
#include "Reflection/ReflectionManager.hpp"

namespace claid
{
    class BinaryDeserializer : public Deserializer<BinaryDeserializer>
    {
    
        private:
            BinaryDataReader binaryDataReader;

        public:
            
            const static std::string getReflectorName()
            {
                return "BinaryDeserializer";
            } 

            BinaryDeserializer()
            {
                
            }

           

            template<typename T>
            void callFloatOrDouble(const char* property, ReflectedVariable<T> member)
            {
                T tmp;
                this->binaryDataReader.read(tmp);
                member = tmp;
            }   

            // Also includes any variants of signed, unsigned, short, long, long long, ...
            template<typename T>
            void callInt(const char* property, ReflectedVariable<T> member)
            {
                T tmp;
                this->binaryDataReader.read(tmp);
                member = tmp;
            }

            void callBool(const char* property, ReflectedVariable<bool> member)
            {
                bool tmp;
                this->binaryDataReader.read(tmp);
                member = tmp;
            }

            // Why template? Because we can have signed and unsigned char.
            template<typename T>
            void callChar(const char* property, ReflectedVariable<T> member)
            {
                T tmp;
                this->binaryDataReader.read(tmp);
                member = tmp;          
            }

            void callString(const char* property, ReflectedVariable<std::string> member)
            {
                std::string tmp;
                this->binaryDataReader.readString(tmp);
                member = tmp;         
            }

            template<typename T>
            void callBeginClass(const char* property, ReflectedVariable<T> member)
            {
                
            }

            template<typename T>
            void callEndClass(const char* property, ReflectedVariable<T> member)
            {

            }


            template<typename T>
            void callPointer(const char* property, ReflectedVariable<T*> member)
            {
                std::string className;
                this->binaryDataReader.readString(className);

                // Check if ClassFactory is registered for className.
                if (!ClassFactory::ClassFactory::getInstance()->isFactoryRegisteredForClass(className))
                {
                    CLAID_THROW(claid::Exception, "BinaryDeserializer failed to deserialize object from binary data. Class \"" << className << "\" was not registered to ClassFactory and is unknown.");
                } 

                member = ClassFactory::ClassFactory::getInstance()->getNewInstanceAndCast<T>(className);

                UntypedReflector* untypedReflector;
                if (!ReflectionManager::getInstance()->getReflectorForClass(className, this->getReflectorName(), untypedReflector))
                {
                    CLAID_THROW(claid::Exception, "BinaryDeserializer failed to deserialize object from binary. Member \"" << property << "\" is a pointer type with it's class specified as \"" << className << "\". However, no PolymorphicReflector was registered for class \"" << className << "\". Was CLAID_SERIALIZATION implemented for this type?");
                }

                untypedReflector->invoke(static_cast<void*>(this), static_cast<void*>(member));               
            }
            
            template<typename T>
            void callSharedPointer(const char* property, ReflectedVariable<T> member)
            {
                typedef typename T::element_type BaseTypeT;
                BaseTypeT* pointer;
                this->callPointer<BaseTypeT>(property, make_reflected_variable(property, pointer));
                member = T(pointer, std::default_delete<BaseTypeT>());
            }

            template<typename T>
            void callEnum(const char* property, ReflectedVariable<T> member)
            {
                int32_t m;
                ReflectedVariable<int32_t> var = make_reflected_variable(property, m);
                this->callInt(property, var);
                member = static_cast<T>(var.get());
            }

            void count(const std::string& name, int32_t& count)
            {
                this->binaryDataReader.read(count);
            }

            void countElements(int32_t& count)
            {
                this->binaryDataReader.read(count);
            }

            void beginSequence()
            {
                // Do nothing
            }

            void itemIndex(const size_t index)
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

            

            template <typename T>
            typename std::enable_if<!std::is_arithmetic<T>::value>::type
            deserialize(T& obj, BinaryDataReader& binaryDataReader)
            {
                this->binaryDataReader = binaryDataReader;

                // Read data type string and check if it matches the data type of obj.
                std::string name = TypeChecking::getCompilerIndependentTypeNameOfClass<T>();
                std::string storedName;


                this->binaryDataReader.readString(storedName);

                if(name != storedName)
                {
                    CLAID_THROW(Exception, "Error, failed to deserialize object from binary data because of mismatching types."
                                                "The data type of the object is " << name << " but the serialized data is of type " << storedName);
                }

                invokeReflectOnObject(obj);
            }

            template <typename T>
            typename std::enable_if<std::is_arithmetic<T>::value>::type
            deserialize(T& obj, BinaryDataReader& binaryDataReader)
            {
                this->binaryDataReader = binaryDataReader;

                // Read data type string and check if it matches the data type of obj.
                std::string name = TypeChecking::getCompilerIndependentTypeNameOfClass<T>();
                std::string storedName;


                this->binaryDataReader.readString(storedName);

                if(name != storedName)
                {
                    CLAID_THROW(Exception, "Error, failed to deserialize object from binary data because of mismatching types."
                                                "The data type of the object is " << name << " but the serialized data is of type " << storedName);
                }

                this->binaryDataReader.read(obj);
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

