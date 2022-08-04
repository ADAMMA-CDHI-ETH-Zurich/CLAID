#pragma once
#include "XMLNode.hpp"
#include "XMLNumericVal.hpp"

#include "Reflection/Deserializer.hpp"

#include "XML/XMLParser.hpp"
#include "ClassFactory/ClassFactory.hpp"
#include "PolymorphicReflector/PolymorphicReflector.hpp"
#include <algorithm>
#include <memory.h>

namespace portaible
{
    class XMLDeserializer : public Deserializer<XMLDeserializer>
    {
        private:
            std::shared_ptr<XMLNode> xmlRootNode;
            std::shared_ptr<XMLNode> currentXMLNode;

            bool getCurrentNodeClassName(std::string& className)
            {
                return this->currentXMLNode->getAttribute("class", className);
            }

            bool isSequence = false;
            size_t idInSequence = 0;

        public:

            XMLDeserializer()
            {

            }

            XMLDeserializer(std::shared_ptr<XMLNode> xmlRootNode) : xmlRootNode(xmlRootNode), currentXMLNode(xmlRootNode)
            {

            }

            template<typename T>
            void callFloatOrDouble(const char* property, T& member)
            {
                std::shared_ptr<XMLNode> node = this->getChildNode(property);
                if (node.get() == nullptr)
                {
                    if (!this->defaultValueCurrentlySet())
                    {
                        PORTAIBLE_THROW(portaible::Exception, "Error during deserialization from XML. XML Node \"" << property << "\" is missing!");
                    }

                    member = this->getCurrentDefaultValue<T>();
                }
                else
                {
                    std::shared_ptr<XMLVal> value = std::static_pointer_cast<XMLVal>(node);

                    if (value.get() == nullptr)
                    {
                        PORTAIBLE_THROW(portaible::Exception, "Error during deserialization from XML. XML Node \"" << property << "\" was expected to be an XMLVal, but apparently it's not. This should be a programming error.");
                    }
                    XMLNumericVal::parseFromString(member, value->getValue());
                }
            }   

            // Also includes any variants of signed, unsigned, short, long, long long, ...
            template<typename T>
            void callInt(const char* property, T& member)
            {
                std::shared_ptr<XMLNode> node = this->getChildNode(property);
                if(node.get() == nullptr)
                {
                    if (!this->defaultValueCurrentlySet())
                    {
                        PORTAIBLE_THROW(portaible::Exception, "Error during deserialization from XML. XML Node \"" << property << "\" is missing!");
                    }

                    member = this->getCurrentDefaultValue<T>();
                }
                else
                {
                    std::shared_ptr<XMLVal> value = std::static_pointer_cast<XMLVal>(node);

                    if (value.get() == nullptr)
                    {
                        PORTAIBLE_THROW(portaible::Exception, "Error during deserialization from XML. XML Node \"" << property << "\" was expected to be an XMLVal, but apparently it's not. This should be a programming error.");
                    }
                    XMLNumericVal::parseFromString(member, value->getValue());
                }
            }

            void callBool(const char* property, bool& member)
            {
                std::shared_ptr<XMLNode> node = this->getChildNode(property);
                if(node.get() == nullptr)
                {
                    if (!this->defaultValueCurrentlySet())
                    {
                        PORTAIBLE_THROW(portaible::Exception, "Error during deserialization from XML. XML Node \"" << property << "\" is missing!");
                    }

                    member = this->getCurrentDefaultValue<bool>();
                }
                else
                {
                    std::shared_ptr<XMLVal> value = std::static_pointer_cast<XMLVal>(node);

                    if (value.get() == nullptr)
                    {
                        PORTAIBLE_THROW(portaible::Exception, "Error during deserialization from XML. XML Node \"" << property << "\" was expected to be an XMLVal, but apparently it's not. This should be a programming error.");
                    }
                    
                    std::string boolStr = value->getValue();
                    // convert string to lower case
                    std::for_each(boolStr.begin(), boolStr.end(), [](char & c) {
                        c = ::tolower(c);
                    });

                    if(boolStr == "true")
                    {
                        member = true;
                    }
                    else if(boolStr == "false")
                    {
                        member = false;
                    }
                    else
                    {
                        PORTAIBLE_THROW(portaible::Exception, "Error during deserialization from XML. Cannot deserialize xml value to member/property \"" << property << "\" of type bool. Expected \"true\" or \"false\" in XML, got " << boolStr << ".");
                    }
                }
            }

            void callChar(const char* property, char& member)
            {
                std::shared_ptr<XMLNode> node = this->getChildNode(property);
                if(node.get() == nullptr)
                {
                    if (!this->defaultValueCurrentlySet())
                    {
                        PORTAIBLE_THROW(portaible::Exception, "Error during deserialization from XML. XML Node \"" << property << "\" is missing!");
                    }

                    member = this->getCurrentDefaultValue<char>();
                }
                else
                {
                    std::shared_ptr<XMLVal> value = std::static_pointer_cast<XMLVal>(node);

                    if (value.get() == nullptr)
                    {
                        PORTAIBLE_THROW(portaible::Exception, "Error during deserialization from XML. XML Node \"" << property << "\" was expected to be an XMLVal, but apparently it's not. This should be a programming error.");
                    }
                   
                    const std::string& str = value->getValue();
                    if(str.size() != 1)
                    {
                        PORTAIBLE_THROW(portaible::Exception, "Error during deserialization from XML. XML Node \"" << property << "\" is a character, however in the XML file either an empty string or a string with more than one character was specified. Got " << str << ".");
                    }

                    member = str[0];
                }
            }

            template<typename T>
            void callBeginClass(const char* property, T& member)
            {
                std::shared_ptr<XMLNode> node = this->currentXMLNode->findChild(property);
                if(node.get() == nullptr)
                {
                    PORTAIBLE_THROW(portaible::Exception, "Error during deserialization from XML. XML Node " << property << " is missing!");
                }

                this->currentXMLNode = node;
            }

            template<typename T>
            void callEndClass(const char* property, T& member)
            {
                this->currentXMLNode = this->currentXMLNode->parent;
            }

            template<typename T>
            void callPointer(const char* property, T*& member)
            {
                std::string className;
                if (!this->getCurrentNodeClassName(className))
                {
                    PORTAIBLE_THROW(portaible::Exception, "XMLDeserializer failed to deserialize object from XML. Member \"" << property << "\" is a pointer type. However, attribute \"class\" was not specified for the XML node. We don't know which class you want!");
                }

                if (!ClassFactory::ClassFactory::getInstance()->isFactoryRegisteredForClass(className))
                {
                    PORTAIBLE_THROW(portaible::Exception, "XMLDeserializer failed to deserialize object from XML. Class \"" << className << "\" was not registered to ClassFactory and is unknown.");
                }

                member = ClassFactory::ClassFactory::getInstance()->getNewInstanceAndCast<T>(className);

                PolymorphicReflector::WrappedReflectorBase<XMLDeserializer>* polymorphicReflector;
                if (!PolymorphicReflector::PolymorphicReflector<XMLDeserializer>::getInstance()->getReflector(className, polymorphicReflector))
                {
                    PORTAIBLE_THROW(portaible::Exception, "XMLDeserializer failed to deserialize object from XML. Member \"" << property << "\" is a pointer type. However, attribute \"class\" was does not have a PolymorphicReflector registered. Was PORTAIBLE_SERIALIZATION implemented for this type?");
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

            template<typename T>
            void callEnum(const char* property, T& member)
            {
                size_t m;
                this->callInt(property, m);
                member = static_cast<T>(m);
            }

            template<typename T>
            void callString(const char* property, T& member)
            {
                std::shared_ptr<XMLNode> node = this->getChildNode(property);
                if(node.get() == nullptr)
                {
                    if (!this->defaultValueCurrentlySet())
                    {
                        PORTAIBLE_THROW(portaible::Exception, "Error during deserialization from XML. XML Node \"" << property << "\" is missing!");
                    }

                    member = this->getCurrentDefaultValue<T>();
                }
                else
                {
                    std::shared_ptr<XMLVal> value = std::static_pointer_cast<XMLVal>(node);

                    if (value.get() == nullptr)
                    {
                        PORTAIBLE_THROW(portaible::Exception, "Error during deserialization from XML. XMLNode was expected to be an XMLVal, but apparently it's not. This should be a programming error.");
                    }
                    member = value->getValue();
                }
            }

            std::shared_ptr<XMLNode> getChildNode(const char* property)
            {
                if(!this->isSequence)
                {
                    return this->currentXMLNode->findChild(property);
                }
                else
                {
                    size_t ctr = 0;
                    for(std::shared_ptr<XMLNode> child : this->currentXMLNode->children)
                    {
                        if(child->name == std::string(property))
                        {
                            if(ctr >= this->idInSequence)
                            {
                                this->idInSequence++;
                                return child;
                            }
                            ctr++;
                        }
                    }
                }

                return nullptr;
            }

            void count(const std::string& name, size_t& count)
            {
                count = 0;
                for(std::shared_ptr<XMLNode> node : this->currentXMLNode->children)
                {
                    if(node->name == name)
                    {
                        count++;
                    }
                }
            }

            void countElements(size_t& count)
            {
                count = this->currentXMLNode->children.size();
            }

            void beginSequence()
            {
                this->isSequence = true;
                this->idInSequence = 0;
            }

            void endSequence()
            {
                this->isSequence = false;
            }

            void write(const char* data, size_t size)
            {

            }

            void read(char*& data, size_t size)
            {
                
            }

            template<typename T> 
            void deserialize(std::string name, T& obj)
            {
                this->currentXMLNode = xmlRootNode;
                
                this->currentXMLNode = this->currentXMLNode->findChild(name);

                if (this->currentXMLNode == nullptr)
                {
                    PORTAIBLE_THROW(portaible::Exception, "Error in deserialization of object of type " << name << " from XML. No XML node corresponding to the object was found (<" << name << "> missing).");
                }
               
               invokeReflectOnObject(obj);

            }

            template<typename T>
            void deserializeFromNode(std::string name, T& obj)
            {
                this->callAppropriateFunctionBasedOnType(name.c_str(), obj);
            }

            void enforceName(std::string& name, int idInSequence = 0)
            {
                // Some serializers, like BinarySerializer, might not store the members name (i.e., property parameters),
                // as it is not necessary to be known in the binary data).
                // For some cases, however, it might be necessary to store such strings in the serialized data nevertheless,
                // that might be needed for deserialization etc.
                // Thus, this function allows to make sure the string "name" is explicitly stored.
                

                // Note, that as the XMLSerializer stores names of members everytime anyways, this function is not needed
                // for XMLSerializer and -Deserialize.

                if(idInSequence < 0 )
                {
                    PORTAIBLE_THROW(Exception, "Error! During Deserialization, enforceName was called with an invalid idInSequence (must be >= 0).");
                }
                else if(idInSequence >= this->currentXMLNode->children.size())
                {
                    PORTAIBLE_THROW(Exception, "Error! During Deserialization, enforceName was called with idInSequence, " 
                    << "which is greater than the number of items in the current sequence (" << idInSequence << " vs. " << this->currentXMLNode->children.size());
                }

                name = this->currentXMLNode->children[idInSequence]->name;
            }

            

        
    };
}