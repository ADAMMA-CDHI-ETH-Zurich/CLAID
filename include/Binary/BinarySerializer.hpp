#pragma once
#include "BinaryData.hpp"
#include "ClassFactory/ClassFactory.hpp"

#include "Serialization/Serializer.hpp"
#include "TypeChecking/TypeCheckingFunctions.hpp"
#include "PolymorphicReflector/PolymorphicReflector.hpp"

namespace claid
{
    class BinarySerializer : public Serializer<BinarySerializer>
    {
    

        public:
            
            BinaryData* binaryData;

            BinarySerializer()
            {
                
            }

            template<typename T>
            void callFloatOrDouble(const char* property, T& member)
            {
                this->binaryData->store(member);
            }   

            // Also includes any variants of signed, unsigned, short, long, long long, ...
            template<typename T>
            void callInt(const char* property, T& member)
            {
                this->binaryData->store(member);
            }

            void callBool(const char* property, bool& member)
            {
                this->binaryData->store(member);
            }

            // Why template? Because we can have signed and unsigned char.
            template<typename T>
            void callChar(const char* property, T& member)
            {
                this->binaryData->store(member);
            }

            void callString(const char* property, std::string& member)
            {
                this->binaryData->storeString(member);
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
                if(member == nullptr)
                {
                    // What to do with nullptrs? Do not serialize?
                    CLAID_THROW(Exception, "Error, BinarySerializer can not serialize member " << property << "."
                    << "The member is a pointer of type " << TypeChecking::getCompilerSpecificCompileTypeNameOfClass<T>() << ", but the value of the pointer is null.");
                    return;
                }

                // In the following, we check whether serialization is implemented for member.
                // Note, that T might not match the members real type, as member might be polymorphic.
                // Thus, we can not use className to check whether a PolymorphicReflector was registered (see below).

                // Note, that this check is only necessary for SERIALIZERS.
                // The serializer needs to make sure it stores the exact className.
                // The deserializer deserializes the data for the object (member) based on this className.
                // Thus, the deserializer relies on the serializer to store the correct className.

                // We only need the rttiType for checking whether the type of member has implemented serialization.
                // Cannot use className to check whether the type is registered to ClassFactory and PolymorphicReflector,
                // because getClassName is a virtual function. If a type is derived from a base class AND 
                // implements serialization (registered to ClassFactory and PolymorphicReflector), then getClassName()
                // provides the correct type. However, if the derived type does NOT implement serialization,
                // getClassName returns the className of the base type, which would lead into storing the wrong
                // class identifier in the binary data.
                std::string rttiTypeString = TypeChecking::getCompilerSpecificRunTimeNameOfObject(*member);

                if(!ClassFactory::ClassFactory::getInstance()->isFactoryRegisteredForRTTITypeName(rttiTypeString))
                {
                    CLAID_THROW(claid::Exception, "BinarySerializer failed to serialize object to binary. Member \"" << property << "\" is a pointer/polymorphic object of RTTI type \"" << rttiTypeString << "\". However, no PolymorphicReflector was registered for type \"" << rttiTypeString << "\". Was PORTAIBLE_SERIALIZATION implemented for this type?");
                }

                // If there is a factory available for data type with given RTTI string (see above), then the className will
                // be the correct name of the Polymorphic class.
                std::string className = member->getClassName();

                PolymorphicReflector::WrappedReflectorBase<BinarySerializer>* polymorphicReflector;
                if (!PolymorphicReflector::PolymorphicReflector<BinarySerializer>::getInstance()->getReflector(className, polymorphicReflector))
                {
                    CLAID_THROW(claid::Exception, "BinarySerializer failed to serialize object to binary. Member \"" << property << "\" is a pointer/polymorphic type with it's class specified as \"" << className << "\". However, no PolymorphicReflector was registered for class \"" << className << "\". Was PORTAIBLE_SERIALIZATION implemented for this type?");
                }

                // Store class name
                this->binaryData->storeString(member->getClassName());

                polymorphicReflector->invoke(*this, static_cast<void*>(member));               
            }
            
            template<typename T>
            void callSharedPointer(const char* property, T& member)
            {
                typedef typename T::element_type BaseTypeT;
                BaseTypeT* ptr = member.get();

                if(ptr == nullptr)
                {
                    // What to do with nullptrs? Do not serialize?
                    CLAID_THROW(Exception, "Error, BinarySerializer can not serialize member " << property << "."
                    << "The member is a shared_ptr of type " << TypeChecking::getCompilerSpecificCompileTypeNameOfClass<T>() << ", but the value of the pointer is null");
                    return;
                }

                this->callPointer<BaseTypeT>(property, ptr);
            }

            template<typename T>
            void callEnum(const char* property, T& member)
            {
                size_t m = static_cast<size_t>(member);
                this->callInt(property, m);
            }

            void count(const std::string& name, size_t& count)
            {
                this->binaryData->store(count);
            }

            void countElements(size_t& count)
            {
                this->binaryData->store(count);
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
            
            void write(const char* data, size_t size)
            {
                this->binaryData->insertBytes(data, size);
            }



            template <typename T>
            typename std::enable_if<!(std::is_arithmetic<T>::value || std::is_same<T, CLAID::byte>::value)>::type
            serialize(T& obj, BinaryData* targetContainer)
            {
                this->binaryData = targetContainer;
                this->binaryData->clear();

                // Store data type string in order to check it during deserialization.
                // This is crucial and needs to platform independent.
                std::string name = TypeChecking::getCompilerIndependentTypeNameOfClass<T>();
                this->binaryData->storeString(name);


                invokeReflectOnObject(obj);
            }

            template <typename T>
            typename std::enable_if<std::is_arithmetic<T>::value || std::is_same<T, CLAID::byte>::value>::type
            serialize(T& obj, BinaryData* targetContainer)
            {
                this->binaryData = targetContainer;
                this->binaryData->clear();

                // Store data type string in order to check it during deserialization.
                // This is crucial and needs to platform independent.
                std::string name = TypeChecking::getCompilerIndependentTypeNameOfClass<T>();
                this->binaryData->storeString(name);


                this->binaryData->store(obj);
            }

            void enforceName(std::string& name, int idInSequence = 0)
            {
                // Some serializers, like BinarySerializer, might not store the members name (i.e., property parameters),
                // as it is not necessary to be known in the binary data).
                // For some cases, however, it might be necessary to store such strings in the serialized data nevertheless,
                // as it might be needed for deserialization etc.
                // Thus, this function allows to make sure the string "name" is explicitly stored.
                
                this->binaryData->storeString(name);
            }

    };
}
