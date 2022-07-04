#pragma once
#include "XMLNode.hpp"
#include "XMLNumericVal.hpp"

#include "Reflection/Serializer.hpp"
#include "RunTime/RunTime.hpp"
#include "TypeChecking/TypeCheckingFunctions.hpp"

namespace portaible
{
    class XMLSerializer : public Serializer<XMLSerializer>
    {
        public:
            XMLNode* root = nullptr;
            XMLNode* currentNode = nullptr;

            XMLSerializer()
            {
                this->root = new XMLNode(nullptr, "root");
                this->currentNode = root;
            }

            template<typename T>
            void callFloatOrDouble(const char* property, T& member)
            {
                this->currentNode->addChild(new XMLNumericVal(this->currentNode, property, XMLNumericVal::toString<T>(member)));
            }   

            // Also includes any variants of signed, unsigned, short, long, long long, ...
            template<typename T>
            void callInt(const char* property, T& member)
            {
                this->currentNode->addChild(new XMLNumericVal(this->currentNode, property, XMLNumericVal::toString<T>(member)));
            }

            void callBool(const char* property, bool& member)
            {
                this->currentNode->addChild(new XMLVal(this->currentNode, property, member ? "true" : "false"));
            }

            template<typename T>
            void callString(const char* property, T& member)
            {
                this->currentNode->addChild(new XMLVal(this->currentNode, property, member));
            }

            template<typename T>
            void callBeginClass(const char* property, T& member)
            {
                XMLNode* node = new XMLNode(currentNode, property);
                this->currentNode->addChild(node);
                this->currentNode = node;
            }

            template<typename T>
            void callEndClass(const char* property, T& member)
            {
                this->currentNode = this->currentNode->parent;
            }


            template<typename T>
            void callPointer(const char* property, T*& member)
            {
                static_assert(has_mem_classFactoryRegistrar<T>::value,
                  "Data type (see above) was reflected as pointer and might get invoked by a Serializer, however"
                  "the data type is not de-/serializable, as it was not registered to ClassFactory. Use DECLARE_SERIALIZATION(DataType)"
                  "and PORTABLE_SERIALIZATION(DataType) accordingly.");

                this->invokeReflectOnObject(*member);
                std::string className = member->getClassName();
                this->currentNode->setAttribute("class", className);
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
                // Do nothing
            }

            void countElements(size_t& count)
            {
                // Do nothing
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
                XMLNode* node = new XMLNode(currentNode, name);
                this->currentNode->addChild(node);
                this->currentNode = node;

                invokeReflectOnObject(obj);
            }

            void enforceName(std::string& name, int idInSequence = 0)
            {
                // Some serializers, like BinarySerializer, might not store the members name (i.e., property parameters),
                // as it is not necessary to be known in the binary data).
                // For some cases, however, it might be necessary to store such strings in the serialized data nevertheless,
                // as it might be needed for deserialization etc.
                // Thus, this function allows to make sure the string "name" is explicitly stored.
                

                // Note, that as the XMLSerializer stores names of members everytime anyways, this function is not needed
                // for XMLSerializer and -Deserialize.
            }

    };
}