#pragma once
#include "XMLNode.hpp"
#include "XMLNumericVal.hpp"
#include "XMLPreprocessor.hpp"

#include "Reflection/RecursiveReflector.hpp"
#include "Exception/Exception.hpp"
#include "Utilities/StringUtils.hpp"
#include <deque>

namespace claid
{
    class XMLParser
    {
        private:
            std::shared_ptr<XMLNode> root = nullptr;
            std::string xmlFilePath = "TestDocument.xml";

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

            bool isAlphaNumericCharacter(char character)
            {
                return std::isalnum(character);
            }

            bool isCommentStart(const std::string& xmlContent, size_t& index)
            {
                // for a comment we require 4 characters: <!--
                const std::string commentTag = "<!--";
                const int numCharacters = commentTag.size();
                if(xmlContent.size() - index < numCharacters)
                {
                    return false;
                }

                return xmlContent.substr(index, numCharacters) == commentTag;
            }

            // The following 3 functions assume that the content of the tag only represents
            // the name of the tag, without whitespaces and attributes.
            // e.g. < testTag class="Test" />  would be testTag/
            bool isOpenTag(const std::string& tagContent)
            {
                // Make sure we dont find a '/' in the tag.
                return tagContent.find("/") == std::string::npos;
            }

            bool isCloseTag(const std::string& tagContent)
            {
                return tagContent[0] == '/';
            }

            bool isOpenCloseTag(const std::string& tagContent)
            {
                // e.g. <Module class="..."/>
                return tagContent[tagContent.size() - 1] == '/';
            }

            void removeWhiteSpaces(std::string& element)
            {
                StringUtils::stringReplaceAll(element, " ", "");
            }

            void removeTabs(std::string& element)
            {
                StringUtils::stringReplaceAll(element, "\t", "");
            }

            void handleComment(const std::string& xmlContent, size_t& index, size_t& lineNumber, size_t& characterIndexInLine)
            {
                size_t commentEndIndex = xmlContent.find("-->", index + 1);

                if(commentEndIndex == std::string::npos)
                {
                    CLAID_THROW(Exception, "Error while parsing XML document \"" << xmlFilePath << "\", found non-ended comment.\n"
                    << "Comment starting in line " << lineNumber << " at character index " << index << ",\n"
                    << "but could not find corresponding comment close.");
                }


                // Skip a head to end of comment.
                // If there is a newline in between, increase lineNumber
                // and rest characterIndex.
                for(; index < commentEndIndex + strlen("-->"); index++)
                {
                    const char& character = xmlContent[index];
                    if(character == '\n')
                    {
                        lineNumber++;
                        characterIndexInLine = 1;
                    }
                    characterIndexInLine++;
                }   
            }

            void parseTag(const std::string& xmlContent, size_t& index, size_t& lineNumber, size_t& characterIndexInLine, std::vector<XMLElement>& elementStack)
            {
                std::string tagContent = "";
                
                // Current index is < 
                index += 1;

                const size_t originalLineNumber = lineNumber;
                const size_t originalCharacterIndexInLine = characterIndexInLine;

                for(; index < xmlContent.size(); index++)
                {
                    characterIndexInLine++;
                     
                    const char& character = xmlContent[index];

                  
                    if(character == '\n')
                    {
                        lineNumber++;
                        characterIndexInLine = 0;
                        continue;
                    }
                    if(character == '<')
                    {
                        // Is it a comment?
                        if(isCommentStart(xmlContent, index))
                        {
                            handleComment(xmlContent, index, lineNumber, characterIndexInLine);
                        }
                        else
                        {
                            // If not, it is an error.
                            CLAID_THROW(Exception, "Error while parsing XML document \"" << xmlFilePath << ",\n"
                            << "Found unexpected \'<\' inside opening tag at line " << lineNumber << " index " << characterIndexInLine);
                        }

                        
                    }
                    else if(character == '>')
                    {
                        // Found end of tag!
                        break;
                    }
               
                 

                    tagContent += character;
                }

                if(tagContent.size() == 0)
                {
                    CLAID_THROW(Exception, "Error while parsing XML document \"" << xmlFilePath << "\".\n"
                    "Found empty tag <> at line " << lineNumber << " index " << characterIndexInLine);
                }

                
                XMLElement xmlTagElement;
        
                xmlTagElement.element = tagContent;
                this->splitElementIntoElementAndAttributes(xmlTagElement, originalLineNumber, originalCharacterIndexInLine);
                
                removeWhiteSpaces(xmlTagElement.element);
                removeTabs(xmlTagElement.element);

                if(xmlTagElement.element.size() == 0)
                {
                    CLAID_THROW(Exception, "Error while parsing XML document \"" << xmlFilePath << "\".\n"
                    "Found empty tag <> or tag that only contains attributes at line " << lineNumber << " index " << characterIndexInLine);
                }



                xmlTagElement.elementType = XMLElementType::OPENING;     

                if(isOpenTag(xmlTagElement.element))
                {
                    xmlTagElement.elementType = XMLElementType::OPENING;   
                    elementStack.push_back(xmlTagElement);  
                }
                else if(isCloseTag(xmlTagElement.element))
                {
                    // Remove the / from the beginning of the elemment content.
                    xmlTagElement.element = xmlTagElement.element.substr(1, xmlTagElement.element.size());
                    xmlTagElement.elementType = XMLElementType::CLOSING;     
                    elementStack.push_back(xmlTagElement);
                }
                else if(isOpenCloseTag(xmlTagElement.element))
                {
                    // Remove the / from the end of the element content.
                    xmlTagElement.element = xmlTagElement.element.substr(0, xmlTagElement.element.size() - 1);

                    xmlTagElement.elementType = XMLElementType::OPENING;
                    elementStack.push_back(xmlTagElement);  

                    XMLElement closingElement;
                    closingElement.element = xmlTagElement.element;
                    closingElement.elementType = XMLElementType::CLOSING;
                    elementStack.push_back(closingElement);     
                }

                printf("%s\n", xmlTagElement.element.c_str());


            }

            void splitElementIntoElementAndAttributes(XMLElement& element, size_t lineNumber, size_t characterIndexInLine)
            {
                std::vector<std::string> tokens;
                std::string currentToken = "";
                bool insideString = false;
                printf("element %s\n", element.element.c_str());
                for(size_t index = 0; index < element.element.size(); index++)
                {
                    characterIndexInLine++;
                    const char& currentCharacter = element.element[index];

                    if(currentCharacter == '\n')
                    {
                        lineNumber += 1;
                        characterIndexInLine = 0;
                        continue;
                    }

                    if(!insideString)
                    {
                        if(currentCharacter == ' ' || currentCharacter == '\t')
                        {
                            if(currentToken.size() > 0)
                            {
                                tokens.push_back(currentToken);
                            }
                            currentToken = "";
                            continue;
                        }

                        if(currentCharacter == '\"')
                        {
                            insideString = true;
                            printf("inside string\n");
                            if(currentToken.size() > 0)
                            {
                                tokens.push_back(currentToken);
                            }
                            tokens.push_back("\"");
                            currentToken = "";
                            continue;
                        }

                        if(currentCharacter == '=')
                        {
                            if(currentToken.size() > 0)
                            {
                                tokens.push_back(currentToken);
                            }
                            tokens.push_back("=");
                            currentToken = "";
                            continue;
                        }

                        if(!isAlphaNumericCharacter(currentCharacter) && currentCharacter != '/')
                        {
                            CLAID_THROW(Exception, "Error while parsing XML document \"" << xmlFilePath << "\".\n"
                            "Found non-alphanumeric character \'" << currentCharacter << "\' at line " << lineNumber << " index " << characterIndexInLine);
                        }
                    }
                    else
                    {
                        if(currentCharacter == '\"')
                        {
                            insideString = false;
                            tokens.push_back(currentToken);
                            tokens.push_back("\"");
                            currentToken = "";
                        }
                    }
                    currentToken += currentCharacter;
                }

                for(const std::string& t : tokens)
                {
                    printf("token %s\n", t.c_str());
                }
            }

            void insertAndClearTokenIfNotEmpty(std::vector<std::string>& tokens, std::string& token)
            {
                if(token.size() > 0 )
                {
                    tokens.push_back(token);
                    token = "";
                }
            }

            void tokenize(const std::string& xmlContent)
            {
                size_t lineNumber = 1; 
                size_t characterIndexInLine = 0;

                std::vector<std::string> tokens;
                std::string currentToken = "";

                bool insideString = false;
                bool insideComment = false;

                for(size_t index = 0; index < xmlContent.size(); index++)
                {
                    const char& currentCharacter = xmlContent[index];
                    characterIndexInLine++;

                    if(currentCharacter == '\n')
                    {
                        lineNumber++;
                        characterIndexInLine = 0;
                        continue;
                    }

                    if(currentCharacter == ' ' || currentCharacter == '\t')
                    {
                        insertAndClearTokenIfNotEmpty(tokens, currentToken);
                        continue;
                    }

                    if(insideString)
                    {
                        if(currentCharacter == '\"')
                        {
                            tokens.push_back(currentToken);
                            tokens.push_back("\"");
                            currentToken = "";
                            insideString = false;
                            continue;
                        }
                    }
                    else
                    {   
                        if(isCommentStart(xmlContent, index))
                        {
                            printf("is comment start %d %d %d\n", index, lineNumber, characterIndexInLine);
                            handleComment(xmlContent, index, lineNumber, characterIndexInLine);
                            printf("is comment start %d %d %d\n", index, lineNumber, characterIndexInLine);
                            continue;
                        }

                        if(currentCharacter == '\"')
                        {
                            insertAndClearTokenIfNotEmpty(tokens, currentToken);
                            tokens.push_back("\"");
                            insideString = true;
                            continue;
                        }

                        if(currentCharacter == '=')
                        {
                            insertAndClearTokenIfNotEmpty(tokens, currentToken);
                            tokens.push_back("=");
                            continue;
                        }

                        if(currentCharacter == '/')
                        {
                            insertAndClearTokenIfNotEmpty(tokens, currentToken);
                            tokens.push_back("/");
                            continue;
                        }

                        if(currentCharacter == '<')
                        {
                            insertAndClearTokenIfNotEmpty(tokens, currentToken);
                            tokens.push_back("<");
                            continue;
                        }

                        if(currentCharacter == '>')
                        {
                            insertAndClearTokenIfNotEmpty(tokens, currentToken);
                            tokens.push_back(">");
                            continue;
                        }

                        if(!isAlphaNumericCharacter(currentCharacter))
                        {
                            CLAID_THROW(Exception, "Error while parsing XML document \"" << xmlFilePath << "\".\n"
                            "Found non-alphanumeric character \'" << currentCharacter << "\' at line " << lineNumber << " index " << characterIndexInLine);
                        }            



                    }
                    


                    currentToken += currentCharacter;
                }

                for(const std::string& t : tokens)
                {
                    printf("token %s\n", t.c_str());
                }
            }

        public:

            void parseXMLString(const std::string& xmlContent)
            {
                size_t lineNumber =  1;
                size_t characterIndexInLine = 0;

                std::vector<XMLElement> stack;

                for(size_t i = 0; i < xmlContent.size(); i++)
                {
                    const char& character = xmlContent[i];
                    

                    characterIndexInLine++;
                    // Those characters are always to be ignored, except if we are inside a string
                    if(character == '\t' || character == ' ')
                    {
                        continue;
                    }

                    if(character == '\n')
                    {
                        characterIndexInLine = 0;
                        lineNumber++;
                        continue;
                    }

                    

                    if(character == '<')
                    {
                        // There can only be 3 cases:
                        // <node>
                        // </node>
                        // Or comments.

                        const char& nextCharacter = xmlContent[i + 1];

                        if(isCommentStart(xmlContent, i))
                        {
                            handleComment(xmlContent, i, lineNumber, characterIndexInLine);
                            continue;
                        }
                        else
                        {
                            // We are in a tag
                            parseTag(xmlContent, i, lineNumber, characterIndexInLine, stack);
                        }
                        
                    }
                    else if(character == '>')
                    {
                        CLAID_THROW(Exception, "Error while parsing XML document \"" << xmlFilePath << "\".\n"
                        "Found end symbol \'>\' but there was no open symbol \'<\' before at line " << lineNumber << " index " << characterIndexInLine);
                    }
                    else if(!isAlphaNumericCharacter(character))
                    {
                        CLAID_THROW(Exception, "Error while parsing XML document \"" << xmlFilePath << "\".\n"
                        "Found non-alphanumeric character \'" << character << "\' at line " << lineNumber << " index " << characterIndexInLine);
                    }
                    


                    
                }
            }

            bool parseFromString(const std::string& string, std::shared_ptr<XMLNode>& rootNode, std::string parentFilePath = "")
            {
                tokenize(string);
                //this->parseXMLString(string);
                return true;
                std::string filtered = string;

                XMLPreprocessor preprocessor;

                // Removes unnecessary whitespaces and tabs and resolves include directives.
                preprocessor.preprocess(filtered, parentFilePath);

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
                //parseToStack(nodeBegin, stack);

                // if(stack[0].element != "root")
                // {
                //     CLAID_THROW(claid::Exception, "Expected first element in XML to be <root>. Root node missing!");
                // }

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
                            // TODO: Check if matches previous open tag

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
                        // TODO: Check if matches previous open tag


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

            // void parseToStack(const std::string& xml, std::vector<XMLElement>& stack)
            // {
            //     size_t index = 0;

            //     while(index < xml.size())
            //     {
        
            //         XMLElement xmlElement;
            //         if(xml.at(index) == '<')
            //         {   
            //             if(!(index + 1 < xml.size()))
            //             {
            //                 CLAID_THROW(claid::Exception, "Error while parsing XML, unexpected < at end of file.");
            //             }

            //             size_t indexTmp = xml.find(">", index);
                    
            //             if(xml.at(index + 1) == '/')
            //             {
            //                 xmlElement.elementType = XMLElementType::CLOSING;
            //                 xmlElement.element = xml.substr(index + 2, indexTmp - index - 2);
            //             }
            //             else
            //             {
            //                 xmlElement.elementType = XMLElementType::OPENING;     
            //                 xmlElement.element = xml.substr(index + 1, indexTmp - index - 1);

            //                 if (xmlElement.element.find("=") != std::string::npos)
            //                 {
            //                     this->splitElementIntoElementAndAttributes(xmlElement);
            //                 }
            //             }
            //             index = indexTmp + 1;

            //         }
            //         else
            //         {   
            //             xmlElement.elementType = XMLElementType::VALUE;
            //             size_t indexTmp = xml.find("</", index);
            //             xmlElement.element = xml.substr(index, indexTmp - index);
            //             index = indexTmp;
            //         }

            //         stack.push_back(xmlElement);

            //     }

                
            // }

            

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




            
        
    };
}