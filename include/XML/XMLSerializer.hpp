#pragma once
#include "XMLNode.hpp"
#include "XMLNumericVal.hpp"

#include "Serialization/Serializer.hpp"
#include "TypeChecking/TypeCheckingFunctions.hpp"

namespace claid
{
    class XMLSerializer : public Serializer<XMLSerializer>
    {
        public:
            std::shared_ptr<XMLNode> root = nullptr;
            std::shared_ptr<XMLNode> currentNode = nullptr;

            XMLSerializer()
            {
                this->root = std::shared_ptr<XMLNode>(new XMLNode(nullptr, "root"));
                this->currentNode = root;
            }

            template<typename T>
            void callFloatOrDouble(const char* property, T& member)
            {
                this->currentNode->addChild(
                    std::static_pointer_cast<XMLNode>(
                        std::shared_ptr<XMLNumericVal>(
                            new XMLNumericVal(this->currentNode, property, XMLNumericVal::toString<T>(member)))));
            }   

            // Also includes any variants of signed, unsigned, short, long, long long, ...
            template<typename T>
            void callInt(const char* property, T& member)
            {
                this->currentNode->addChild(
                    std::static_pointer_cast<XMLNode>(
                        std::shared_ptr<XMLNumericVal>(
                            new XMLNumericVal(this->currentNode, property, XMLNumericVal::toString<T>(member)))));
            }

            void callBool(const char* property, bool& member)
            {
                this->currentNode->addChild(
                    std::static_pointer_cast<XMLNode>(
                        std::shared_ptr<XMLVal>(
                            new  XMLVal(this->currentNode, property, member ? "true" : "false"))));
            }

            // Why template? Because we can have signed and unsigned char.
            template<typename T>
            void callChar(const char* property, T& member)
            {
                std::string str = "";
                str += member;

                this->currentNode->addChild(
                    std::static_pointer_cast<XMLNode>(
                        std::shared_ptr<XMLVal>(
                            new XMLVal(this->currentNode, property, str))));
            }

            template<typename T>
            void callString(const char* property, T& member)
            {
                this->currentNode->addChild(
                    std::static_pointer_cast<XMLNode>(
                        std::shared_ptr<XMLVal>(
                            new XMLVal(this->currentNode, property, member))));
            }

            template<typename T>
            void callBeginClass(const char* property, T& member)
            {
                std::shared_ptr<XMLNode> node = std::shared_ptr<XMLNode>(new XMLNode(currentNode, property));
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
                if(member == nullptr)
                {
                    // What to do with nullptrs? Do not serialize?
                    PORTAIBLE_THROW(Exception, "Error, XMLSerializer can not serialize member " << property << "."
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
                    PORTAIBLE_THROW(claid::Exception, "XMLSerializer failed to serialize object to XML. Member \"" << property << "\" is a pointer/polymorphic object of type \"" << rttiTypeString << "\". However, no PolymorphicReflector was registered for type \"" << rttiTypeString << "\". Was PORTAIBLE_SERIALIZATION implemented for this type?");
                }

                std::string className = member->getClassName();

                PolymorphicReflector::WrappedReflectorBase<XMLSerializer>* polymorphicReflector;
                if (!PolymorphicReflector::PolymorphicReflector<XMLSerializer>::getInstance()->getReflector(className, polymorphicReflector))
                {
                    PORTAIBLE_THROW(claid::Exception, "XMLSerializer failed to deserialize object from XML. Member \"" << property << "\" is a pointer type with it's class specified as \"" << className << "\". However, no PolymorphicReflector was registered for class \"" << className << "\". Was PORTAIBLE_SERIALIZATION implemented for this type?");
                }

                polymorphicReflector->invoke(*this, static_cast<void*>(member));
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
                    PORTAIBLE_THROW(Exception, "Error, XMLSerializer can not serialize member " << property << "."
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

            }

            template<typename T> 
            void serialize(T& obj)
            {
                std::string name = TypeChecking::getCompilerSpecificCompileTypeNameOfClass<T>();
                std::shared_ptr<XMLNode> node = std::shared_ptr<XMLNode>(new XMLNode(currentNode, name));
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