#pragma once
#include "XMLNode.hpp"
#include "XMLNumericVal.hpp"

#include "Serialization/Deserializer.hpp"

#include "XML/XMLParser.hpp"
#include "ClassFactory/ClassFactory.hpp"
#include "Reflection/Reflect.hpp"
#include "Reflection/ReflectionManager.hpp"
#include <algorithm>
#include <memory.h>
#include <stack>

namespace claid
{
    class XMLDeserializer : public Deserializer<XMLDeserializer>
    {
        private:
            std::shared_ptr<XMLNode> xmlRootNode;
            std::shared_ptr<XMLNode> currentXMLNode;

            std::stack<std::shared_ptr<XMLNode>> nodeStack;

            bool getCurrentNodeClassName(std::string& className)
            {
                return this->currentXMLNode->getAttribute("class", className);
            }

            bool isSequence = false;
            size_t idInSequence = 0;


        public:

            std::string getReflectorName()
            {
                return "XMLDeserializer";
            } 

            XMLDeserializer()
            {

            }

            XMLDeserializer(std::shared_ptr<XMLNode> xmlRootNode) : xmlRootNode(xmlRootNode), currentXMLNode(xmlRootNode)
            {

            }

            EmptyReflect(XMLDeserializer)

            template<typename T>
            void callFloatOrDouble(const char* property, T& member)
            {

                std::shared_ptr<XMLNode> node = this->getChildNode(property);
                if (node.get() == nullptr)
                {
                    if (!this->defaultValueCurrentlySet())
                    {
                        CLAID_THROW(claid::Exception, "Error during deserialization from XML. XML Node \"" << property << "\" of member \"" << this->getDebugNodeName(this->currentXMLNode) << "\" is missing!");
                    }

                    assignDefaultValue(member);
                }
                else
                {
                    std::shared_ptr<XMLVal> value = std::static_pointer_cast<XMLVal>(node);

                    if (value.get() == nullptr)
                    {
                        CLAID_THROW(claid::Exception, "Error during deserialization from XML. XML Node \"" << property << "\" was expected to be an XMLVal, but apparently it's not. This should be a programming error.");
                    }
                    XMLNumericVal::parseFromString(property, member, value->getValue());
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
                        CLAID_THROW(claid::Exception, "Error during deserialization from XML. XML Node \"" << property << "\" of member \"" << getDebugNodeName(this->currentXMLNode) << "\" is missing!");
                    }

                    assignDefaultValue(member);
                }
                else
                {
                    std::shared_ptr<XMLVal> value = std::static_pointer_cast<XMLVal>(node);

                    if (value.get() == nullptr)
                    {
                        CLAID_THROW(claid::Exception, "Error during deserialization from XML. XML Node \"" << property << "\" was expected to be an XMLVal, but apparently it's not. This should be a programming error.");
                    }
                    XMLNumericVal::parseFromString(property, member, value->getValue());
                }
            }

            void callBool(const char* property, bool& member)
            {
                std::shared_ptr<XMLNode> node = this->getChildNode(property);
                if(node.get() == nullptr)
                {
                    if (!this->defaultValueCurrentlySet())
                    {
                        CLAID_THROW(claid::Exception, "Error during deserialization from XML. XML Node \"" << property << "\" of member \"" << this->getDebugNodeName(this->currentXMLNode) << "\" is missing!");
                    }

                    assignDefaultValue(member);
                }
                else
                {
                    std::shared_ptr<XMLVal> value = std::static_pointer_cast<XMLVal>(node);

                    if (value.get() == nullptr)
                    {
                        CLAID_THROW(claid::Exception, "Error during deserialization from XML. XML Node \"" << property << "\" was expected to be an XMLVal, but apparently it's not. This should be a programming error.");
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
                        CLAID_THROW(claid::Exception, "Error during deserialization from XML. Cannot deserialize xml value to member/property \"" << property << "\" of type bool. Expected \"true\" or \"false\" in XML, got " << boolStr << ".");
                    }
                }
            }

            // char, signed char, unsigned char (char can either be defined as signed char or as unsigned char)
            // See: https://stackoverflow.com/questions/16503373/difference-between-char-and-signed-char-in-c   
            template<typename T>
            void callChar(const char* property, T& member)
            {
                std::shared_ptr<XMLNode> node = this->getChildNode(property);
                if(node.get() == nullptr)
                {
                    if (!this->defaultValueCurrentlySet())
                    {
                        CLAID_THROW(claid::Exception, "Error during deserialization from XML. XML Node \"" << property << "\" of member \"" << this->getDebugNodeName(this->currentXMLNode) << "\" is missing!");
                    }

                    assignDefaultValue(member);
                }
                else
                {
                    std::shared_ptr<XMLVal> value = std::static_pointer_cast<XMLVal>(node);

                    if (value.get() == nullptr)
                    {
                        CLAID_THROW(claid::Exception, "Error during deserialization from XML. XML Node \"" << property << "\" was expected to be an XMLVal, but apparently it's not. This should be a programming error.");
                    }
                   
                    const std::string& str = value->getValue();
                    if(str.size() != 1)
                    {
                        CLAID_THROW(claid::Exception, "Error during deserialization from XML. XML Node \"" << property << "\" is a character, however in the XML file either an empty string or a string with more than one character was specified. Got " << str << ".");
                    }

                    member = str[0];
                }
            }

            template<typename T>
            void callBeginClass(const char* property, T& member)
            {
                std::shared_ptr<XMLNode> node = getChildNode(property);
                if(node.get() == nullptr)
                {
                    if (!this->defaultValueCurrentlySet())
                    {
                        std::string className = TypeChecking::getCompilerSpecificRunTimeNameOfObject(member);
                        CLAID_THROW(claid::Exception, "Error during deserialization from XML. XML Node \"" << property << "\" of member \"" << this->getDebugNodeName(this->currentXMLNode) << "\" is missing!"
                        "We tried to deserialize an object of class \"" << className << "\", which has a member \"" << property << "\". This member was not specified in the corresponding XML node.");
                    }
                    assignDefaultValue(member);
                }

                this->nodeStack.push(this->currentXMLNode);

                this->currentXMLNode = node;
                this->isSequence = false;
            }

            template<typename T>
            void callEndClass(const char* property, T& member)
            {
                this->currentXMLNode = this->nodeStack.top();
                this->nodeStack.pop();
            }

            template<typename T>
            void callPointer(const char* property, T*& member)
            {
                std::shared_ptr<XMLNode> node = this->getChildNode(property);

                if (node.get() == nullptr)
                {
                    if (!this->defaultValueCurrentlySet())
                    {
                        CLAID_THROW(claid::Exception, "Error during deserialization from XML. XML Node \"" << property << "\" of member \"" << this->getDebugNodeName(this->currentXMLNode) << "\" is missing!");
                    }

                    assignDefaultValue(member);

                    return;
                }
               

                std::string className;
                
                if (!node->getAttribute("class", className))
                {
                    CLAID_THROW(claid::Exception, "XMLDeserializer failed to deserialize object from XML. Member \"" << property << "\" is a pointer type. However, attribute \"class\" was not specified for the XML node. We don't know what class to instantiate.");
                }

                if (!ClassFactory::ClassFactory::getInstance()->isFactoryRegisteredForClass(className))
                {
                    CLAID_THROW(claid::Exception, "XMLDeserializer failed to deserialize object from XML. Class \"" << className << "\" was not registered to ClassFactory and is unknown.");
                }

                member = ClassFactory::ClassFactory::getInstance()->getNewInstanceAndCast<T>(className);

                UntypedReflector* untypedReflector;
                if (!ReflectionManager::getInstance()->getReflectorForClass(className, this->getReflectorName(), untypedReflector))
                {
                    CLAID_THROW(claid::Exception, "XMLDeserializer failed to deserialize object from XML. Member \"" << property << "\" is a polymorphic pointer type of class specified as \"" << className << "\". However, no PolymorphicReflector was registered for class \"" << className << "\". Was PORTAIBLE_SERIALIZATION implemented for this type?");
                }
                std::shared_ptr<XMLNode> parent = this->currentXMLNode;
                this->currentXMLNode = node;
                untypedReflector->invoke(static_cast<void*>(this), static_cast<void*>(member));               
                this->currentXMLNode = parent;
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
                int32_t m;
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
                        CLAID_THROW(claid::Exception, "Error during deserialization from XML. XML Node \"" << property << "\" of member \"" << this->getDebugNodeName(this->currentXMLNode) << "\" is missing!");
                    }

                    assignDefaultValue(member);
                }
                else
                {
                    std::shared_ptr<XMLVal> value = std::static_pointer_cast<XMLVal>(node);

                    if (value.get() == nullptr)
                    {
                        CLAID_THROW(claid::Exception, "Error during deserialization from XML. XMLNode was expected to be an XMLVal, but apparently it's not. This should be a programming error.");
                    }
                    member = value->getValue();
                }
            }

            std::shared_ptr<XMLNode> getChildNode(const char* property)
            {
                if(this->currentXMLNode == nullptr)
                {
                    return nullptr;
                }
                
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
                                return child;
                            }
                            ctr++;
                        }
                    }
                }

                return nullptr;
            }

            void count(const std::string& name, int32_t& count)
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

            void countElements(int32_t& count)
            {
                count = this->currentXMLNode->children.size();
            }

            void beginSequence()
            {
                this->isSequence = true;
                this->idInSequence = 0;
            }

            void itemIndex(const size_t index)
            {
                this->idInSequence = index;

                // If you have a nested container, e.g. vector<vector<int>>, then 
                // isSequence would be set to false when endSequence of the inner vector is called.
                // But, as we call itemIndex for each item, we can just set it back to true, if we're still in a sequence.
                this->isSequence = true;
            }

            void endItem()
            {
                
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
                    CLAID_THROW(claid::Exception, "Error in deserialization of object of type " << name << " from XML. No XML node corresponding to the object was found (<" << name << "> missing).");
                }
               
               invokeReflectOnObject(name.c_str(), obj);

            }

            template<typename T>
            void deserializeFromNode(std::string name, T& obj)
            {
                std::shared_ptr<XMLNode> newRootNode = std::make_shared<XMLNode>(nullptr, name);
                newRootNode->addChild(this->currentXMLNode);
                this->xmlRootNode = newRootNode;
                this->currentXMLNode = this->xmlRootNode;
   

                this->member(name.c_str(), obj, "");
            }

            template<typename T>
            void deserializeExistingPolymorphicObject(std::string className, T* obj)
            {
                // Assumes T is a polymorphic type (e.g., any class inheriting from claid::Module) and
                // calls the corresponding PolymorphicReflector on that object.
                // The object needs to exist already! deserializeExistingPolymorphicObject will NOT create the object using the ClassFactory!


                if(obj == nullptr)
                {
                    CLAID_THROW(claid::Exception, "Error in deserialization from XML. It was tried to deserialize a polymorphic object with specified class \"" << className << "\", however the "
                    "object is null. The object needs to have been created before being able to deserialize it's members from XML."
                    "If you want the deserializer to automatically create a corresponding object using the ClassFactory, use the deserialize function instead.");
                }

                UntypedReflector* untypedReflector;
                if (!ReflectionManager::getInstance()->getReflectorForClass(className, this->getReflectorName(), untypedReflector))
                {
                    CLAID_THROW(claid::Exception, "XMLDeserializer failed to deserialize object from XML. No PolymorphicReflector was registered for class \"" << className << "\". Was CLAID_SERIALIZATION implemented for this type?");
                }

                untypedReflector->invoke(static_cast<void*>(this), static_cast<void*>(obj));
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
                    CLAID_THROW(Exception, "Error! During Deserialization, enforceName was called with an invalid idInSequence (must be >= 0).");
                }
                else if(idInSequence >= this->currentXMLNode->children.size())
                {
                    CLAID_THROW(Exception, "Error! During Deserialization, enforceName was called with idInSequence, " 
                    << "which is greater than the number of items in the current sequence (" << idInSequence << " vs. " << this->currentXMLNode->children.size());
                }

                name = this->currentXMLNode->children[idInSequence]->name;
            }

            std::string getDebugNodeName(std::shared_ptr<XMLNode> node)
            {
                if(node->hasAttribute("class"))
                {
                    std::string className;
                    node->getAttribute("class", className);
                    return node->name + std::string(" (class: ") + className + std::string(")");
                }
                else
                {
                    return node->name;
                }
            }

            bool setByteRepresentationOfSerializedData(std::vector<char>& data)
            {
                return false;
            }
        
    };
}


