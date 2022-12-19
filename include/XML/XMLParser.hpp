#pragma once
#include "XMLNode.hpp"
#include "XMLNumericVal.hpp"

#include "Reflection/RecursiveReflector.hpp"

#include "Exception/Exception.hpp"

#include <deque>

namespace claid
{
    class XMLParser
    {
        private:
            std::shared_ptr<XMLNode> root = nullptr;

            enum XMLElementType
            {
                OPENING,
                VALUE,
                CLOSING
            };

            struct XMLElement
            {
                XMLElementType elementType;
                std::string element;
                std::map<std::string, std::string> attributes;
            };



        public:
            bool parseFromString(const std::string& string, std::shared_ptr<XMLNode>& rootNode)
            {
                std::string filtered = string;

                replaceInString(filtered, "\n", "");
                replaceInString(filtered, "> ", ">");
                replaceInString(filtered, "< ", "");
                replaceInString(filtered, "\t", "");

                removeWhiteSpacesBetweenTabs(filtered);
                removeWhiteSpacesAtBeginning(filtered);


                if(filtered.size() == 0)
                {
                    CLAID_THROW(Exception, "Cannot parse XML config, config is empty or only contains whitespaces!");
                    return false;
                }


                if(!this->parseNode(filtered, rootNode))
                {
                    return false;
                }

                
                return true;   
            }

            bool parseNode(const std::string& nodeBegin, std::shared_ptr<XMLNode>& rootNode)
            {
                // Should be end of data.
                if(nodeBegin == "")
                {
                    return true;
                }

                std::vector<XMLElement> stack;
                std::vector<std::shared_ptr<XMLNode>> parentStack;
                parseToStack(nodeBegin, stack);

                if(stack[0].element != "root")
                {
                    CLAID_THROW(claid::Exception, "Expected first element in XML to be <root>. Root node missing!");
                }

                rootNode = std::shared_ptr<XMLNode>(new XMLNode(nullptr, "root"));
                
                std::shared_ptr<XMLNode> currentNode = rootNode;
                

                
                for(size_t i = 1; i < stack.size() - 1; i++)
                {
                    const XMLElement& element = stack[i];

  

                    if(element.elementType == XMLElementType::OPENING)
                    {

                        XMLElement nextXmlElement = stack[i + 1];
                       

                        if(nextXmlElement.elementType == XMLElementType::OPENING)
                        {
                            // Means we have a child.

                            parentStack.push_back(currentNode);

                            std::shared_ptr<XMLNode> newNode = std::shared_ptr<XMLNode>(new XMLNode(currentNode, element.element));
                            newNode->attributes = element.attributes;
                            currentNode->addChild(newNode);
                            currentNode = newNode;
                        }
                        else if(nextXmlElement.elementType == XMLElementType::VALUE)
                        {
                            parentStack.push_back(currentNode);

                            // We are a value.
                            std::shared_ptr<XMLNode> newNode = std::static_pointer_cast<XMLNode>(std::shared_ptr<XMLVal>(new XMLVal(currentNode, element.element, nextXmlElement.element)));
                            currentNode->addChild(newNode);
                            i++;
                        }
                        else if (nextXmlElement.elementType == XMLElementType::CLOSING)
                        {
                            // This means we are an empty node.
                            parentStack.push_back(currentNode);

                            std::shared_ptr<XMLNode> newNode = std::shared_ptr<XMLNode>(new XMLNode(currentNode, element.element));
                            newNode->attributes = element.attributes;
                            currentNode->addChild(newNode);
                            currentNode = newNode;
                        }
                    }
                    else if (element.elementType == XMLElementType::CLOSING)
                    {
                        currentNode = parentStack.back();
                        parentStack.pop_back();
                    }
                    else
                    {
                        CLAID_THROW(claid::Exception, "Invalid XML! Found a value where a tag (opening or closing) was expected.");
                    }
                }

                

                return true;
            }

            void parseToStack(const std::string& xml, std::vector<XMLElement>& stack)
            {
                size_t index = 0;

            


                while(index < xml.size())
                {
        

                    XMLElement xmlElement;
                    if(xml.at(index) == '<')
                    {   
                        if(!(index + 1 < xml.size()))
                        {
                            CLAID_THROW(claid::Exception, "Error while parsing XML, unexpected < at end of file.");
                        }

                        size_t indexTmp = xml.find(">", index);
                    
                        if(xml.at(index + 1) == '/')
                        {
                            xmlElement.elementType = XMLElementType::CLOSING;
                            xmlElement.element = xml.substr(index + 2, indexTmp - index - 2);
                        }
                        else
                        {
                            xmlElement.elementType = XMLElementType::OPENING;     
                            xmlElement.element = xml.substr(index + 1, indexTmp - index - 1);

                            if (xmlElement.element.find("=") != std::string::npos)
                            {
                                this->splitElementIntoElementAndAttributes(xmlElement);
                            }
                        }
                        index = indexTmp + 1;

                    }
                    else
                    {   
                        xmlElement.elementType = XMLElementType::VALUE;
                        size_t indexTmp = xml.find("</", index);
                        xmlElement.element = xml.substr(index, indexTmp - index);
                        index = indexTmp;
                    }

                    stack.push_back(xmlElement);

                }

                
            }

            void splitElementIntoElementAndAttributes(XMLElement& element)
            {
                std::string xmlTag = element.element;
                size_t index = xmlTag.find(" ");
                element.element = xmlTag.substr(0, index);

                replaceInString(xmlTag, "\"", "");
                
                std::string attributeName;
                std::string attributeValue;

                while (index < xmlTag.size())
                {
                    attributeName = "";
                    attributeValue = "";
                    while (xmlTag.at(index) == ' ')
                    {
                        index++;
                    }

                    while (xmlTag.at(index) != ' ' && xmlTag.at(index) != '=')
                    {
                        attributeName += xmlTag.at(index);
                        index++;
                    }

                    while (xmlTag.at(index) == ' ' || xmlTag.at(index) == '=')
                    {
                        index++;

                    }

                    while (xmlTag.at(index) == ' ')
                    {
                        index++;
                    }

                    while (index < xmlTag.size() && xmlTag.at(index) != ' ')
                    {
                        attributeValue += xmlTag.at(index);
                        index++;
                    }

                    element.attributes.insert(std::make_pair(attributeName, attributeValue));
                }               
            }

            void getNextElementAndReduce(const std::string& begin, XMLElement& xmlElement, std::string& reduced)
            {
            
                size_t index;
                if(begin.at(0) == '<' && begin.at(1) == '/')
                {
                    xmlElement.elementType = XMLElementType::CLOSING;
                    index = begin.find(">");
                    xmlElement.element = begin.substr(2, index + 1 - 2);
                }
                else if(begin.at(0) == '<')
                {   
                    xmlElement.elementType = XMLElementType::OPENING;
                    index = begin.find(">");
                    xmlElement.element = begin.substr(1, index + 1 - 1);
                }
                else
                {   
                    xmlElement.elementType = XMLElementType::VALUE;
                    index = begin.find("<");
                    reduced = begin.substr(0, index);
                }
            }


            bool getOpeningTag(const std::string& tagBegin, std::string& tag)
            {
                if(tagBegin.at(0) != '<')
                {
                    CLAID_THROW(Exception, "Error, expected opening tag.");
                    return false;
                }

                size_t index = tagBegin.find(">");
                tag = tagBegin.substr(0, index + 1);

                return true;


            }

            void replaceInString(std::string& subject, const std::string& search,
                          const std::string& replace) 
            {
                size_t pos = 0;
                while((pos = subject.find(search, pos)) != std::string::npos)
                {
                    subject.replace(pos, search.length(), replace);
                    pos += replace.length();
                }
            }



            void removeWhiteSpacesBetweenTabs(std::string& string)
            {
                std::string copy = string;

                size_t index = 0;

                string = "";

                while (index < copy.size())
                {
                    string += copy.at(index);
                    if (copy.at(index) == '>')
                    {
                        index++;
                        while (index < copy.size() && copy.at(index) == ' ')
                        {
                            index++;
                        }
                        index--;
                     
                    }
                    index++;
                }
            }

            void removeWhiteSpacesAtBeginning(std::string& string)
            {
                int charIndex = 0;
                while(string[charIndex] == ' ' && charIndex < string.size())
                {
                    charIndex++;
                }

                string = string.substr(charIndex, string.size());
            }
        
    };
}