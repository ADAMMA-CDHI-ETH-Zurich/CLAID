#pragma once
#include "BinaryData.hpp"

#include "Reflection/Serializer.hpp"
#include "RunTime/RunTime.hpp"
#include "TypeChecking/TypeCheckingFunctions.hpp"

namespace portaible
{
    class BinarySerializer : public Serializer<BinarySerializer>
    {
    

        public:
            
            BinaryData binaryData;

            BinarySerializer()
            {
                
            }

            template<typename T>
            void callFloatOrDouble(const char* property, T& member)
            {
                this->binaryData.store(member);
            }   

            // Also includes any variants of signed, unsigned, short, long, long long, ...
            template<typename T>
            void callInt(const char* property, T& member)
            {
                this->binaryData.store(member);
            }

            void callBool(const char* property, bool& member)
            {
                this->binaryData.store(member);
            }

            void callString(const char* property, std::string& member)
            {
                this->binaryData.storeString(member);
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
                static_assert(has_mem_classFactoryRegistrar<T>::value,
                  "Data type (see above) was reflected as pointer and might get invoked by a Serializer, however"
                  "the data type is not de-/serializable, as it was not registered to ClassFactory. Use DECLARE_SERIALIZATION(DataType)"
                  "and PORTABLE_SERIALIZATION(DataType) accordingly.");

                // Store class name
                this->binaryData.storeString(member->getClassName());
                this->invokeReflectOnObject(*member);                
            }
            
            template<typename T>
            void callSharedPointer(const char* property, T& member)
            {
                typedef typename T::element_type BaseTypeT;
                BaseTypeT* ptr = member.get();


                if(ptr == nullptr)
                {
                    // What to do with nullptrs? Do not serialize?
                    return;
                }

                this->callPointer<BaseTypeT>(property, ptr);
            }

            void count(const std::string& name, size_t& count)
            {
                this->binaryData.store(count);
            }

            void countElements(size_t& count)
            {
                this->binaryData.store(count);
            }

            void beginSequence()
            {
                // Do nothing
            }

            void endSequence()
            {
                // Do nothing
            }
            

            template<typename T> 
            void serialize(T& obj)
            {
                std::string name = portaible::getDataTypeRTTIString<T>();

                invokeReflectOnObject(obj);
            }

            void enforceName(std::string& name, int idInSequence = 0)
            {
                // Some serializers, like BinarySerializer, might not store the members name (i.e., property parameters),
                // as it is not necessary to be known in the binary data).
                // For some cases, however, it might be necessary to store such strings in the serialized data nevertheless,
                // as it might be needed for deserialization etc.
                // Thus, this function allows to make sure the string "name" is explicitly stored.
                
                this->binaryData.store(name);
            }

    };
}