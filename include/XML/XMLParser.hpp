#pragma once
#include "XMLNode.hpp"
#include "XMLNumericVal.hpp"
#include "XMLPreprocessor.hpp"

#include "Reflection/RecursiveReflector.hpp"
#include "Exception/Exception.hpp"
#include "Utilities/StringUtils.hpp"
#include "Utilities/Path.hpp"
#include <deque>

namespace claid
{
    class XMLParser
    {
        private:
            std::shared_ptr<XMLNode> root = nullptr;
            std::string xmlFilePath = "";

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
                        if(isCommentStart(xmlContent, index))
                        {
                            handleComment(xmlContent, index, lineNumber, characterIndexInLine);
                            index--;
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
                
                // If there is a token at the end of a file, we also have to insert it.
                // This is the case if currentToken is not empty after we finished the loop.
                insertAndClearTokenIfNotEmpty(tokens, currentToken, lineNumber, characterIndexInLine);
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
                                        << "At line "  << tokens[i + 2].lineNumber << " index " << tokens[i + 2].characterIndexInLine);
                                    }

                                    if(closingQuotationMark != "\"")
                                    {
                                        CLAID_THROW(Exception, "Error while parsing XML document \"" << xmlFilePath << "\".\n"
                                        "Tried to parse attribute \"" << tokens[i].token << "\"  of tag \"" << elementName << "\". Expected '\"' but found '" << closingQuotationMark << "'"
                                        << "At line "  << tokens[i + 3].lineNumber << " index " << tokens[i + 3].characterIndexInLine);
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
                            xmlElements.push_back(xmlValueElement);
                        }
                     
                      

                    }
                
                }

            }

            void resolveIncludes(const std::vector<XMLElement>& xmlElements, std::vector<XMLElement>& updatedXMLElements)
            {
                // This will always iterate over all elements, even if there is no include statement.
                // However, it will resolve includes in linear time, since include statements found in included
                // files will also be resolved within this loop. 

                for(size_t i = 0; i < xmlElements.size(); i++)
                {
                    const XMLElement& element = xmlElements[i];
                    if(element.elementType == XMLElementType::OPENING && element.element == "include")
                    {
                        if(i + 2 >= xmlElements.size())
                        {
                            CLAID_THROW(Exception, "Error while parsing XML document \"" << xmlFilePath << "\".\n"
                            <<"Failed to resolve include statement. Found <include>, but were either missing include path or </include> tag.\n"
                            << "Specify include statements as follows: <include>PATH/TO/FILE</include>"
                            << "At line " << element.lineNumber << " index " << element.characterIndexInLine);
                        }
                        const XMLElement includeValue = xmlElements[i + 1];
                        const XMLElement includeCloseTag = xmlElements[i + 2];

                        if(includeValue.elementType != XMLElementType::VALUE)
                        {
                            CLAID_THROW(Exception, "Error while parsing XML document \"" << xmlFilePath << "\".\n"
                            <<"Failed to resolve include statement. Found <include>, but no include path afterward was provided\n"
                            << "Specify include statements as follows: <include>PATH/TO/FILE</include>"
                            << "At line " << element.lineNumber << " index " << element.characterIndexInLine);
                        }

                        if(includeCloseTag.elementType != XMLElementType::CLOSING || includeCloseTag.element != "include")
                        {
                            CLAID_THROW(Exception, "Error while parsing XML document \"" << xmlFilePath << "\".\n"
                            <<"Failed to resolve include statement. Found <include>, but no corresponding closing tag </include>\n"
                            << "Specify include statements as follows: <include>PATH/TO/FILE</include>"
                            << "At line " << element.lineNumber << " index " << element.characterIndexInLine);
                        }

                        std::string includePath = includeValue.element;
                        std::string folderPathOfCurrentFile = Path(this->xmlFilePath).getFolderPath();
                        if(!this->isAbsolutePath(includePath))
                        {
                            if(folderPathOfCurrentFile != "")
                            {
                                includePath = Path::join(folderPathOfCurrentFile, includePath).toString();
                            }
                        }

                        std::string includedContent;
                        if(!this->loadFileToString(includePath, includedContent))
                        {
                            CLAID_THROW(Exception, "Failed to resolve include directive in XML document. File \"" << includePath << "\" cannot be read.");
                        }
                        printf("Included content: %s\n", includedContent.c_str());

                        std::vector<XMLElement> tmpElements;

                        XMLParser subParser(includePath);
                        subParser.xmlContentToXMLElements(includedContent, tmpElements);
                        updatedXMLElements.insert(updatedXMLElements.end(), tmpElements.begin(), tmpElements.end());
                        i += 2;
                    }   
                    else
                    {
                        updatedXMLElements.push_back(element);
                    }
                }                
            }


            bool loadFileToString(const std::string& filePath, std::string& fileContent)
            {
                std::ifstream file(filePath);
                fileContent = "";

                printf("Loading file %s\n", filePath.c_str());
                if (!file.is_open())
                {
                    Logger::printfln("Error! Could not open file \"%s\".", filePath.c_str());
                    return false;
                }

                file.seekg(0, std::ios::end);
                fileContent.reserve(file.tellg());
                file.seekg(0, std::ios::beg);

                fileContent.assign((std::istreambuf_iterator<char>(file)),
			        std::istreambuf_iterator<char>());
            
                return true;
            }


            bool isAbsolutePath(const std::string& path)
            {
                if(path.size() == 0)
                {
                    return false;
                }

                #ifdef _WIN32
                    if(path.size() < 4)
                    {
                        return false;
                    }
                    
                    return (path[1] == ':' && path[2] == '/' && path[3] == '/');
                #else
                    return path[0] == '/';
                #endif
            }

            void getDirectoryPathFromFilePath(std::string& path)
            {
                printf("Get folder from path %s\n", path.c_str());
                size_t index = path.size() - 1;
                bool pathSeparatorFound = false;
                while(index > 0)
                {
                    if(path[index] == '/' || path[index] == '\\')
                    {
                        pathSeparatorFound = true;
                        break;
                    }
                    index--;
                }

                if(pathSeparatorFound)
                {
                    path = path.substr(0, index);
                }
                else
                {
                    // Must be a relative path?
                    path = ".";
                }
            }

            // Returns true if any of the XMLElements in elements is of type XMLValue.
            // In this case, the first found element will be copied to foundElement.
            bool getValueElement(const std::vector<XMLElement>& elements, XMLElement& foundElement)
            {
                for(const XMLElement& element : elements)
                {
                    if(element.elementType == XMLElementType::VALUE)
                    {
                        foundElement = element;
                        return true;
                    }
                }
                return false;
            }

        public:
            XMLParser()
            {

            }

            XMLParser(const std::string& xmlFilePath) : xmlFilePath(xmlFilePath)
            {

            }

            void xmlContentToXMLElements(const std::string& xmlContent, std::vector<XMLElement>& xmlElements)
            {
                // Exceptioins will be thrown by the functions used in the following, if XML content is invalid.

                std::vector<TokenElement> tokens;
                tokenize(xmlContent, tokens);

                std::vector<XMLElement> tmpElements;
                buildXMLElementsStackFromTokens(tokens, tmpElements);

                resolveIncludes(tmpElements, xmlElements);
            }

            bool parseFromString(const std::string& string, std::shared_ptr<XMLNode>& rootNode, std::string parentFilePath = "")
            {
                std::vector<XMLElement> stack;
                xmlContentToXMLElements(string, stack);

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

                size_t startIndex = 0;
                for(; startIndex < stack.size(); startIndex++)
                {
                    const XMLElement& element = stack[startIndex];
                    if(element.elementType == XMLElementType::EMPTY)
                    {
                        continue;
                    }

                    // Check that first non-empty element is an opening tag 
                    if(element.elementType == XMLElementType::OPENING)
                    {
                        break;
                    }
                    else
                    {
                        CLAID_THROW(Exception, "Error while parsing XML document \"" << xmlFilePath << "\".\n"
                        << "Expected first tag in document to be an opening tag, but found closing tag or element \"" << element.element << "\"\n" 
                        << "At line "  << element.lineNumber << " index " << element.characterIndexInLine);
                    }
                }


                std::vector<XMLElement> intermediateElements;

                const XMLElement* lastOpeningNode = nullptr;

                for(size_t i = startIndex; i < stack.size() - 1; i++)
                {
                    const XMLElement& element = stack[i];

                    if(element.elementType == XMLElementType::OPENING)
                    {
                        openAndCloseTagsStack.push(element);

                        // Whenever we encounter an opening node, the node can either become a 
                        // node having a value, such as <MyInt>42</MyInt>, or a node having children like
                        // <MyObject><MyInt>42</MyInt></MyObject>.
                        // Hence, whether we have to create a XMLNode or a XMLValueNode can only be decided the next time
                        // when we enocunter an opening or closing tag.
                        // Why do we need to do it this way, that we have to wait for the next element?
                        // Because our XMLTree is not well implemented..
                        // A node in an XML tree should have a member "value", this value could either be a (list of) children, or a int, float, string etc. value.
                        // However, here our XML tree works a bit different. Each XMLNode has a list of children (vector), and the XMLNodes are polymorphic.
                        // If a XMLNode is a value, it has no Children but is of type XMLVal. 
                        // Hence, in the algorithm below, we have to distinguish whether we have to create a node that has children, or a node that is a XMLVal.
                        // We can only deduce that if we look at the element after the current element..
                        // Maybe this needs improvement in the future. The reason the XML tree was implemented this way is because this makes the implementation of XMLSerializer
                        // and XMLDeserializer a bit easier, as we can check whether a node is a value or not.

                        if(lastOpeningNode != nullptr)
                        {
                            XMLElement foundValueElement;
                            if(getValueElement(intermediateElements, foundValueElement))
                            {
                                CLAID_THROW(Exception, "Error while parsing XML document \"" << xmlFilePath << "\".\n"
                                << "Unexpected element " << foundValueElement.element << "inside of XML element \"<" << lastOpeningNode->element << ">\n" 
                                << "At line "  << foundValueElement.lineNumber << " index " << foundValueElement.characterIndexInLine);
                            }
                            else
                            {
                                std::shared_ptr<XMLNode> newNode(new XMLNode(currentNode, lastOpeningNode->element));
                                newNode->attributes = lastOpeningNode->attributes;
                                currentNode->addChild(newNode);
                                parentStack.push(currentNode);  
                                currentNode = newNode;
                            }
                        }
                        lastOpeningNode = &element;
                        intermediateElements.clear();
                    }
                    else if(element.elementType == XMLElementType::CLOSING)
                    {
                        const XMLElement& previousOpeningOrClosingElement = openAndCloseTagsStack.top();
                        if(element.element != previousOpeningOrClosingElement.element)
                        {
                            CLAID_THROW(Exception, "Error while parsing XML document \"" << xmlFilePath << "\".\n"
                            "Found closing tag \"" << element.element << "\", but expected closing tag \"" << previousOpeningOrClosingElement.element << "\".\n" 
                            << "At line "  << element.lineNumber << " index " << element.characterIndexInLine);
                        }

                        if(lastOpeningNode != nullptr)
                        {
                            // lastOpeningNode should be the same as openAndCloseTagsStack.top(), but just to make sure:
                            if(element.element != lastOpeningNode->element)
                            {
                                CLAID_THROW(Exception, "Error while parsing XML document \"" << xmlFilePath << "\".\n"
                                "Found closing tag \"" << element.element << "\", but expected closing tag \"" << lastOpeningNode->element << "\".\n" 
                                << "At line "  << element.lineNumber << " index " << element.characterIndexInLine);
                            }

                            // We opened a node before, and now we found the matching closing tag.
                            // Therefore, the elements between the openingNode and the current element have
                            // to be of type EMPTY or VALUE, hence we have to create a ValueNode for the lastOpeningNode.

                            // Merge all empty or value elements into one string.
                            std::string elementString;
                            for(const XMLElement& intermediateElement : intermediateElements)
                            {
                                if(intermediateElement.elementType != XMLElementType::VALUE && 
                                        intermediateElement.elementType != XMLElementType::EMPTY)
                                {
                                    CLAID_THROW(Exception, "Error while parsing XML document \"" << xmlFilePath << "\".\n"
                                    "Found unexpected tag \"" << intermediateElement.element << "\", inside of XML element <" << lastOpeningNode->element << ">.\n" 
                                    << "At line "  << intermediateElement.lineNumber << " index " << intermediateElement.characterIndexInLine);
                                }

                                elementString += intermediateElement.element;
                            }

                            // Insert value node.
                            std::shared_ptr<XMLNode> newNode = std::static_pointer_cast<XMLNode>(std::shared_ptr<XMLVal>(new XMLVal(currentNode, lastOpeningNode->element, elementString)));
                            newNode->attributes = lastOpeningNode->attributes;
                            currentNode->addChild(newNode);
                            parentStack.push(currentNode);  
                            currentNode = newNode;

                            intermediateElements.clear();
                        }
                    
                        currentNode = parentStack.top();
                        parentStack.pop();
                        openAndCloseTagsStack.pop();

                        lastOpeningNode = nullptr;
                    }
                    else if(element.elementType == XMLElementType::VALUE || XMLElementType::EMPTY)
                    {
                        intermediateElements.push_back(element);
                    }                    
                }

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