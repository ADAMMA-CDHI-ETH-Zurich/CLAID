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

            struct TokenElement
            {
                size_t lineNumber;
                size_t characterIndexInLine;
                std::string token;

                TokenElement()
                {

                }

                TokenElement(const std::string& token, size_t lineNumber, size_t characterIndexInLine) :
                    token(token), lineNumber(lineNumber), characterIndexInLine(characterIndexInLine)
                {

                }
            };

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

            bool isValidXMLElementCharacter(char character)
            {
                std::vector<char> supportedCharacters = {'_', '-', '.'};

                return isAlphaNumericCharacter(character) || 
                    std::find(supportedCharacters.begin(), supportedCharacters.end(), character) != supportedCharacters.end();
            }

            bool isValidXMLElementName(const std::string& str)
            {
                for(const char& character : str)
                {
                    if(!isValidXMLElementCharacter(character))
                    {
                        return false;
                    }
                }
                return true;
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


            void insertAndClearTokenIfNotEmpty(std::vector<TokenElement>& tokens, TokenElement& token, const size_t lineNumber, const size_t characterIndexInLine)
            {
                if(token.token.size() > 0 )
                {
                    tokens.push_back(token);
                    token = TokenElement("", lineNumber, characterIndexInLine);
                }
            }

            int numTokensLeft(size_t index, std::vector<TokenElement>& tokens)
            {
                return tokens.size() - index - 1;
            }

            void tokenize(const std::string& xmlContent, std::vector<TokenElement>& tokens)
            {
                size_t lineNumber = 1; 
                size_t characterIndexInLine = 0;

                TokenElement currentToken("", lineNumber, characterIndexInLine);

                bool insideString = false;
                bool insideComment = false;

                int numberOfUnclosedTags = 0;

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

                    if(insideString)
                    {
                        if(currentCharacter == '\"')
                        {
                            tokens.push_back(currentToken);
                            tokens.push_back(TokenElement("\"", lineNumber, characterIndexInLine));
                            currentToken = TokenElement("", lineNumber, characterIndexInLine);
                            insideString = false;
                            continue;
                        }
                    }
                    else
                    {   
                        if(currentCharacter == ' ' || currentCharacter == '\t')
                        {
                            insertAndClearTokenIfNotEmpty(tokens, currentToken, lineNumber, characterIndexInLine);
                            
                            if(numberOfUnclosedTags == 0)
                            {
                                // This means we are currently not inside of any tag, but between tags.
                                // Between tags, there could be strings, e.g.: <Module>This is just a test.</Module>
                                // Thus, we also store the tabs and whitespaces as tokens.
                                std::string t(1, currentCharacter);
                                tokens.push_back(TokenElement(t, lineNumber, characterIndexInLine));
                            }
                            
                            continue;
                        }
                    
                        if(isCommentStart(xmlContent, index))
                        {
                            handleComment(xmlContent, index, lineNumber, characterIndexInLine);
                            continue;
                        }

                        if(currentCharacter == '\"')
                        {
                            insertAndClearTokenIfNotEmpty(tokens, currentToken, lineNumber, characterIndexInLine);
                            tokens.push_back(TokenElement("\"", lineNumber, characterIndexInLine));
                            insideString = true;
                            continue;
                        }

                        if(currentCharacter == '=')
                        {
                            insertAndClearTokenIfNotEmpty(tokens, currentToken, lineNumber, characterIndexInLine);
                            tokens.push_back(TokenElement("=", lineNumber, characterIndexInLine));
                            continue;
                        }

                        if(currentCharacter == '/')
                        {
                            insertAndClearTokenIfNotEmpty(tokens, currentToken, lineNumber, characterIndexInLine);
                            tokens.push_back(TokenElement("/", lineNumber, characterIndexInLine));
                            continue;
                        }

                        if(currentCharacter == '<')
                        {
                            numberOfUnclosedTags++;
                            insertAndClearTokenIfNotEmpty(tokens, currentToken, lineNumber, characterIndexInLine);
                            tokens.push_back(TokenElement("<", lineNumber, characterIndexInLine));
                            continue;
                        }

                        if(currentCharacter == '>')
                        {
                            numberOfUnclosedTags--;
                            insertAndClearTokenIfNotEmpty(tokens, currentToken, lineNumber, characterIndexInLine);
                            tokens.push_back(TokenElement(">", lineNumber, characterIndexInLine));
                            continue;
                        }

                        if(!isValidXMLElementCharacter(currentCharacter))
                        {
                            CLAID_THROW(Exception, "Error while parsing XML document \"" << xmlFilePath << "\".\n"
                            "Found non-alphanumeric character \'" << currentCharacter << "\' at line " << lineNumber << " index " << characterIndexInLine);
                        }            



                    }
                    


                    currentToken.token += currentCharacter;
                }

                for(const TokenElement& t : tokens)
                {
                    printf("token %s\n", t.token.c_str());
                }
            }

    

            void buildXMLElementsStackFromTokens(std::vector<TokenElement>& tokens, std::vector<XMLElement>& xmlElements)
            {
                std::vector<std::string> openingTokens;
                for(size_t i = 0; i < tokens.size(); i ++)
                {
                    if(tokens[i].token == "<")
                    {
                        // Its closing token
                        if(numTokensLeft(i, tokens) == 0)
                        {
                            CLAID_THROW(Exception, "Error while parsing XML document \"" << xmlFilePath << "\".\n"
                            "Found opening token \'<\' at the end of file, but no other tokens afterward (unclosed tag) at line " << tokens[i].lineNumber << " index " << tokens[i].characterIndexInLine);
                        }
                        if(tokens[i + 1].token == "/")
                        {
                            if(numTokensLeft(i + 1, tokens) < 2)
                            {
                                 CLAID_THROW(Exception, "Error while parsing XML document \"" << xmlFilePath << "\".\n"
                                "Found closing token \'</\' at the end of file, but no other tokens afterward (unclosed tag) at line " << tokens[i].lineNumber << " index " << tokens[i].characterIndexInLine);
                            }


                            if(tokens[i + 3].token != ">")
                            {
                                CLAID_THROW(Exception, "Error while parsing XML document \"" << xmlFilePath << "\".\n"
                                "Found \'" << tokens[i + 3].token << "\' but expected \">\" (closing tag)" << "\' at line " << tokens[i + 3].lineNumber << " index " << tokens[i + 3].characterIndexInLine);
                            }

                            XMLElement element;
                            element.element = tokens[i + 2].token;
                            element.elementType = XMLElementType::CLOSING;
                            xmlElements.push_back(element);
                        }
                        else
                        {
                            // Its opening token

                            // Next should be the token name e.g., Module (<Module>)

                            if(numTokensLeft(i, tokens) == 0)
                            {
                                CLAID_THROW(Exception, "Error while parsing XML document \"" << xmlFilePath << "\".\n"
                                "Found opening token \'<\' at the end of file, but no other tokens afterward (unclosed tag) at line " << tokens[i].lineNumber << " index " << tokens[i].characterIndexInLine);
                            }
                            i++;

                            std::string elementName = tokens[i].token;
                            if(!isValidXMLElementName(elementName))
                            {
                                // This should have been checked by XML tokenizer already.. but trust no one! Not even yourself!
                                CLAID_THROW(Exception, "Error while parsing XML document \"" << xmlFilePath << "\".\n"
                                "XML tag \"" << elementName << "\" contains invalid characters, see line " << tokens[i].lineNumber << " index " << tokens[i].characterIndexInLine);
                            }
                            i++;

                            XMLElement openingElement;
                            openingElement.element = elementName;
                            
                            while(i < tokens.size())
                            {
                                printf("%s\n", tokens[i].token.c_str());
                                if(tokens[i].token == "/")
                                {
                                    // Is open close tag, e.g.: <Module/>
                                    if(numTokensLeft(i, tokens) == 0)
                                    {
                                        CLAID_THROW(Exception, "Error while parsing XML document \"" << xmlFilePath << "\".\n"
                                        "Found standalone token token \'<.../' at the end of file, but no other tokens afterward (unclosed tag), missing '>' at line " << tokens[i].lineNumber << " index " << tokens[i].characterIndexInLine);
                                    }

                                    if(tokens[i + 1].token != ">")
                                    {
                                        CLAID_THROW(Exception, "Error while parsing XML document \"" << xmlFilePath << "\".\n"
                                        "Found unexpected token '" << tokens[i + 1].token << "', expected '>' " << tokens[i].lineNumber << " index " << tokens[i].characterIndexInLine);
                                    }

                                    // We found something like <Module/> which is equivalent to <Module></Module>, hence we add both to the stack

                                    openingElement.elementType = XMLElementType::OPENING;
                                    xmlElements.push_back(openingElement);

                                    XMLElement closingElement;
                                    closingElement.element = elementName;
                                    closingElement.elementType = XMLElementType::CLOSING;
                                    xmlElements.push_back(closingElement);
                                    break;
                                }
                                else if(tokens[i].token == ">")
                                {
                                    // is regular close tag
                                    printf("Pushing %s\n", openingElement.element.c_str());
                                    openingElement.elementType = XMLElementType::OPENING;
                                    xmlElements.push_back(openingElement);
                                    break;
                                }
                                else
                                {
                                    // Must be attributes
                                    // We must have at least 4 tokens, after the current one:
                                    // e.g. class="TestModule" would be tokens: class, =, ", TestModule, "
                                    std::string attributeName = tokens[i].token;
                                    if(numTokensLeft(i, tokens) < 4)
                                    {
                                        CLAID_THROW(Exception, "Error while parsing XML document \"" << xmlFilePath << "\".\n"
                                        "Tried to parse attribute \"" << tokens[i].token << "\"  of tag \"" << elementName << "\", but attribute content was not specified, or missing quotation marks.\n"
                                        << "At line "  << tokens[i].lineNumber << " index " << tokens[i].characterIndexInLine);
                                    }

                                    std::string equalSign = tokens[i + 1].token;
                                    std::string openingQuotationMark = tokens[i + 2].token;
                                    std::string attributeContent = tokens[i + 3].token;
                                    std::string closingQuotationMark = tokens[i + 4].token;

                                    if(equalSign != "=")
                                    {
                                        CLAID_THROW(Exception, "Error while parsing XML document \"" << xmlFilePath << "\".\n"
                                        "Tried to parse attribute \"" << tokens[i].token << "\"  of tag \"" << elementName << "\". Expected '=' but found '" << equalSign << "'"
                                        << "At line "  << tokens[i + 1].lineNumber << " index " << tokens[i + 1].characterIndexInLine);
                                    }

                                    if(openingQuotationMark != "\"")
                                    {
                                        CLAID_THROW(Exception, "Error while parsing XML document \"" << xmlFilePath << "\".\n"
                                        "Tried to parse attribute \"" << tokens[i].token << "\"  of tag \"" << elementName << "\". Expected '\"' but found '" << openingQuotationMark << "'"
                                        << "At line"  << tokens[i + 2].lineNumber << " index " << tokens[i + 2].characterIndexInLine);
                                    }

                                    if(closingQuotationMark != "\"")
                                    {
                                        CLAID_THROW(Exception, "Error while parsing XML document \"" << xmlFilePath << "\".\n"
                                        "Tried to parse attribute \"" << tokens[i].token << "\"  of tag \"" << elementName << "\". Expected '\"' but found '" << closingQuotationMark << "'"
                                        << "At line"  << tokens[i + 3].lineNumber << " index " << tokens[i + 3].characterIndexInLine);
                                    }
                                    openingElement.attributes.insert(make_pair(attributeName, attributeContent));
                                    i += 5;
                                    continue;
                                }
                                i++;
                            }
                         
                        }
                        
                    }
                    else
                    {

                    }
                
                }
                for(XMLElement& element : xmlElements)
                {
                    std::cout << "Element: " << element.element << " type " << element.elementType << "\n";
                    std::cout << "\tAttributes:\n";
                    for(auto& entry : element.attributes)
                    {
                        std::cout << entry.first << "=" << entry.second << "\n";
                    }
                }
            }

        public:

            

            bool parseFromString(const std::string& string, std::shared_ptr<XMLNode>& rootNode, std::string parentFilePath = "")
            {
                std::vector<TokenElement> tokens;
                tokenize(string, tokens);

                std::vector<XMLElement> stack;
                buildXMLElementsStackFromTokens(tokens, stack);



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