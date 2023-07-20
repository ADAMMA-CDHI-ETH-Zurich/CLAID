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
                CLOSING,
                EMPTY
            };

            struct XMLElement
            {
                XMLElementType elementType;
                std::string element;
                std::map<std::string, std::string> attributes;
                size_t lineNumber;
                size_t characterIndexInLine;
            };

            bool isAlphaNumericCharacter(char character)
            {
                return std::isalnum(character);
            }

            bool isValidXMLElementCharacter(char character)
            {
                std::vector<char> supportedCharacters = {'_', '-'};

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

            bool doesOnlyContainTabsOrWhitespace(const std::string& str)
            {
                for(size_t i = 0; i < str.size(); i++)
                {
                    if(str[i] != ' ' && str[i] != '\t')
                    {
                        return false;
                    }
                }
                return true;
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

                        if(numberOfUnclosedTags == 0)
                        {
                            // This means we are currently not inside of any tag, but between tags.
                            // Between tags, there could be strings, e.g.: <Module>This is just a test.</Module>
                            // Thus, we astore all elements into one token until we find < or >
                            
                        }
                        else
                        {
                            if(currentCharacter == ' ' || currentCharacter == '\t')
                            {
                                insertAndClearTokenIfNotEmpty(tokens, currentToken, lineNumber, characterIndexInLine);
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
                        }

                          

                        // if(!isValidXMLElementCharacter(currentCharacter))
                        // {
                        //     CLAID_THROW(Exception, "Error while parsing XML document \"" << xmlFilePath << "\".\n"
                        //     "Found non-alphanumeric character \'" << currentCharacter << "\' at line " << lineNumber << " index " << characterIndexInLine);
                        // }            



                    }
                    


                    currentToken.token += currentCharacter;
                }

                for(const TokenElement& t : tokens)
                {
                    printf("token %s (%lu)\n", t.token.c_str(), t.token.size());
                }
            }

    

            void buildXMLElementsStackFromTokens(std::vector<TokenElement>& tokens, std::vector<XMLElement>& xmlElements)
            {
                std::vector<std::string> openingTokens;
                for(size_t i = 0; i < tokens.size(); i ++)
                {
                                                    printf("%s\n", tokens[i].token.c_str());

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
                                "Found closing token \'</\' at the end of file which has not been closed or is empty, at line " << tokens[i].lineNumber << " index " << tokens[i].characterIndexInLine);
                            }

                            if(tokens[i + 3].token != ">")
                            {
                                CLAID_THROW(Exception, "Error while parsing XML document \"" << xmlFilePath << "\".\n"
                                "Found \'" << tokens[i + 3].token << "\' but expected \">\" (closing tag)" << "\' at line " << tokens[i + 3].lineNumber << " index " << tokens[i + 3].characterIndexInLine);
                            }

                            XMLElement element;
                            element.element = tokens[i + 2].token;
                            element.elementType = XMLElementType::CLOSING;
                            element.lineNumber = tokens[i].lineNumber; // i and not i + 2, because we take line number of opening tag
                            element.characterIndexInLine = tokens[i].characterIndexInLine;
                            xmlElements.push_back(element);
                            i += 3;
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
                                    openingElement.lineNumber = tokens[i].lineNumber; // i and not i + 2, because we take line number of opening tag
                                    openingElement.characterIndexInLine = tokens[i].characterIndexInLine;
                                    xmlElements.push_back(openingElement);

                                    XMLElement closingElement;
                                    closingElement.element = elementName;
                                    closingElement.elementType = XMLElementType::CLOSING;
                                    closingElement.lineNumber = tokens[i].lineNumber; // i and not i + 2, because we take line number of opening tag
                                    closingElement.characterIndexInLine = tokens[i].characterIndexInLine;
                                    xmlElements.push_back(closingElement);
                                    break;
                                }
                                else if(tokens[i].token == ">")
                                {
                                    // is regular open tag
                                    printf("Pushing %s\n", openingElement.element.c_str());
                                    openingElement.elementType = XMLElementType::OPENING;
                                    openingElement.lineNumber = tokens[i].lineNumber; // i and not i + 2, because we take line number of opening tag
                                    openingElement.characterIndexInLine = tokens[i].characterIndexInLine;
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
                        // We are between tags, hence we have an XML value or empty space.
                 
                        XMLElement xmlValueElement;
                        xmlValueElement.element = tokens[i].token;
                        xmlValueElement.lineNumber = tokens[i].lineNumber; 
                        xmlValueElement.characterIndexInLine = tokens[i].characterIndexInLine;

                        if(!doesOnlyContainTabsOrWhitespace(xmlValueElement.element))
                        {
                            xmlValueElement.elementType = XMLElementType::VALUE;
                            xmlElements.push_back(xmlValueElement);
                        }
                        else
                        {
                            xmlValueElement.elementType = XMLElementType::EMPTY;

                            // We do not need to store any empty elements at the beginning of the file.
                            if(xmlElements.size() > 0)
                            {
                                xmlElements.push_back(xmlValueElement);
                            }
                        }
                     
                      

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

                for(XMLElement& element : xmlElements)
                {
                    if(element.elementType == XMLElementType::OPENING)
                    {
                        std::cout << "<" << element.element << ">\n";
                    }
                    else if(element.elementType == XMLElementType::CLOSING)
                    {
                        std::cout << "</" << element.element << ">\n";
                    }
                    else
                    {
                        std::cout << element.element << "\n";
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

                return generateXMLTreeFromElementsStack(stack, rootNode);
            }

            bool generateXMLTreeFromElementsStack(std::vector<XMLElement>& stack, std::shared_ptr<XMLNode>& rootNode)
            {
                rootNode = std::shared_ptr<XMLNode>(new XMLNode(nullptr, "root"));
                
                if(stack.size() == 0)
                {
                    return true;
                }

                std::shared_ptr<XMLNode> currentNode = rootNode;

                // Stores previous parents.
                // If we spawn a child, it becomes the current parent.
                // If the child ends, we have to be able to retrieve the previous parent (of that child).
                // Since XMLNodes have do NOT store a reference to the parent internally (to avoid cyclic dependencies of the shared_ptrs),
                // we use this stack instead, to retrieve the parents. 
                std::stack<std::shared_ptr<XMLNode>> parentStack;

                // We use this to keep track of opened and closed tags.
                // This allows us to identify unclosed tags.
                std::stack<XMLElement> openAndCloseTagsStack;


                const XMLElement& firstElement = stack[0];
                // Check that firstElement is an opening tag "root"
                // Note, that we have ensured before that the first element of the 
                // stack is not an empty element. 
                if(firstElement.elementType != XMLElementType::OPENING || firstElement.element != "root")
                {
                    CLAID_THROW(Exception, "Error while parsing XML document \"" << xmlFilePath << "\".\n"
                    << "Expected first tag in document to be opening tag <root>, but found \"" << firstElement.element << "\"\n" 
                    << "At line"  << firstElement.lineNumber << " index " << firstElement.characterIndexInLine);
                }




                // Start at 1, because first is <root>
                for(size_t i = 1; i < stack.size() - 1; i++)
                {
                    const XMLElement& element = stack[i];
                    printf("Element %s %d\n", element.element.c_str(), element.elementType);

                    if(element.elementType == XMLElementType::OPENING)
                    {
                        std::shared_ptr<XMLNode> newNode;

                        openAndCloseTagsStack.push(element);
                        parentStack.push(currentNode);

                        // Check if next XMLElement is a value element.
                        // This means that newNode wll be a XMLVal node.
                        if(i + 1 < stack.size())
                        {
                            const XMLElement& nextElement = stack[i + 1];
                            if(nextElement.elementType == XMLElementType::VALUE)
                            {
                                newNode = std::static_pointer_cast<XMLNode>(std::shared_ptr<XMLVal>(new XMLVal(currentNode, element.element, nextElement.element)));
                                newNode->attributes = element.attributes;
                                currentNode->addChild(newNode);
                                i ++;
                            }
                            else if(nextElement.elementType == XMLElementType::EMPTY)
                            {
                                // EMPTY can become a value, if the previous tag was an opening tag, and the next tag is a matching closing tag.
                                // E.g.:
                                // <Module>         </Module>
                                // Otherwise, EMPTY elements are ignored.

                                // Currently, i is an opening tag and i + 1 is an empty element.
                                // Empty can only become a value, if the element after it is a closing tag to fullfill the case described in the comment above.
                                if(i + 2 < stack.size())
                                {
                                    const XMLElement& nextNextElement = stack[i + 2];

                                    if(nextElement.elementType == XMLElementType::OPENING && nextNextElement.elementType == XMLElementType::CLOSING)
                                    {
                                        if(nextElement.element == nextNextElement.element)
                                        {
                                            // The element is a string containing only whitespaces or tabs, hence insert a node.
                                            newNode = std::static_pointer_cast<XMLNode>(std::shared_ptr<XMLVal>(new XMLVal(currentNode, element.element, nextElement.element)));
                                            newNode->attributes = element.attributes;
                                            currentNode->addChild(newNode);
                                            i ++;
                                        }
                                    }
                                }
                                
                            }
                        }
                        
                        if(newNode.get() == nullptr)
                        {
                            newNode = std::shared_ptr<XMLNode>(new XMLNode(currentNode, element.element));
                            newNode->attributes = element.attributes;
                            currentNode->addChild(newNode);
                            currentNode = newNode;
                        }


                    }
            
                    else if (element.elementType == XMLElementType::CLOSING)
                    {
                        const XMLElement& previousOpeningOrClosingElement = openAndCloseTagsStack.top();
                        if(element.element != previousOpeningOrClosingElement.element)
                        {
                            CLAID_THROW(Exception, "Error while parsing XML document \"" << xmlFilePath << "\".\n"
                            "Found closing tag \"" << element.element << "\", but expected closing tag \"" << previousOpeningOrClosingElement.element << "\".\n" 
                            << "At line"  << element.lineNumber << " index " << element.characterIndexInLine);
                        }

                        currentNode = parentStack.top();
                        parentStack.pop();

                        openAndCloseTagsStack.pop();
                    }
                    
                }

                std::string str;
                rootNode->toString(str);
                printf("%s\n", str.c_str());
                return true;
            }



            
        
    };
}

// while(i < tokens.size())
// {
//     xmlValue += tokens[i].token;
//     i++;

//     if(tokens[i].token == "<")
//     {
//         // Because we are in the for loop above, i will be increased again.
//         i--;
//         break;
//     }
// }