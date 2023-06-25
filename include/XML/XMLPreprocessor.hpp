#pragma once

#include "Utilities/StringUtils.hpp"

namespace claid
{
    class XMLPreprocessor
    {
        private:
            struct IncludeDirective
            {
                // Character index of <include> (pointing to <)
                size_t startIndex;

                // Character index of the end of </include> (pointing to >).
                size_t endIndex;
                std::string filePath;
            };

            const std::string INCLUDE_DIRECTIVE_TAG = "include";

        public:
            void preprocess(std::string& xmlString, std::string parentFilePath="")
            {
                removeComments(xmlString);
                StringUtils::stringReplaceAll(xmlString, "\n", "");
                StringUtils::stringReplaceAll(xmlString, "> ", ">");
                StringUtils::stringReplaceAll(xmlString, "< ", "<");
    
                StringUtils::stringReplaceAll(xmlString, "\t", "");

                removeWhiteSpacesBetweenTabs(xmlString);
                removeWhiteSpacesAtBeginning(xmlString);


                if(xmlString.size() == 0)
                {
                    CLAID_THROW(Exception, "Cannot parse XML config, config is empty or only contains whitespaces! File path: \"" << parentFilePath << "\"");
                }

                std::string parentDirectoryPath = parentFilePath;
                getDirectoryPathFromFilePath(parentDirectoryPath);
                                printf("parent directory path %s\n", parentDirectoryPath.c_str());

                resolveIncludeDirectories(xmlString, parentDirectoryPath);

                
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

            void resolveIncludeDirectories(std::string& xmlContent, std::string parentFolderPath = "")
            {
                IncludeDirective includeDirective;

                while(parseNextIncludeDirective(xmlContent, includeDirective))
                {
                    printf("Found include directive %zu %zu %s\n", includeDirective.startIndex, includeDirective.endIndex, includeDirective.filePath.c_str());
                    std::string includePath = includeDirective.filePath;

                    if(!this->isAbsolutePath(includePath))
                    {
                        includePath = parentFolderPath + "/" + includePath;
                    }

                

                    std::string includedContent;
                    if(!this->loadFileToString(includePath, includedContent))
                    {
                        CLAID_THROW(Exception, "Failed to resolve include directive in XML document. File \"" << includePath << "\" cannot be read.");
                    }
                    printf("Included content: %s\n", includedContent.c_str());
                    this->preprocess(includedContent, includePath);

                    this->replaceIncludeDirectiveWithContent(xmlContent, includedContent, includeDirective.startIndex, includeDirective.endIndex);
                    

                }
            }

            bool parseNextIncludeDirective(const std::string& xmlContent, IncludeDirective& includeDirective)
            {
                // At this point, xmlContent is already assumed to be preprocessed.
                // Hence, there should not be any unnecessary whitespaces anymore, hopefully.
                std::string includeOpenTag = "<" + INCLUDE_DIRECTIVE_TAG + ">";
                std::string includeCloseTag = "</" + INCLUDE_DIRECTIVE_TAG + ">";
                size_t beginIndex = xmlContent.find(includeOpenTag);

                if(beginIndex == std::string::npos)
                {
                    return false;
                }

                size_t endIndex = xmlContent.find(includeCloseTag, beginIndex);

                if(endIndex == std::string::npos)
                {
                    CLAID_THROW(Exception, "Error, found opening tag \"" << includeOpenTag << "\" at character index " << beginIndex << " for include directive in XML document, but no corresponding closing tag \"" << includeCloseTag << "\".\n"
                    << "Please check your XML document for validity: \n"
                    << xmlContent);
                }

                includeDirective.filePath = "";
                printf("%zu %zu", beginIndex, endIndex);
                for(size_t i = beginIndex + includeOpenTag.size(); i < endIndex; i++)
                {
                    includeDirective.filePath += xmlContent[i];
                }

                // Set endIndex to the end of </include>
                endIndex += includeCloseTag.size();

                includeDirective.startIndex = beginIndex;
                includeDirective.endIndex = endIndex;

                return true;    
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

            void replaceIncludeDirectiveWithContent(std::string& xmlContent, const std::string& includedContent, size_t includeStartIndex, size_t includeEndIndex)
            {
                if(includeEndIndex <= includeStartIndex)
                {
                    CLAID_THROW(Exception, "Cannot insert content from included XML file. The start index of the include directive is smaller or equal to the end index: " << includeStartIndex << " vs. " << includeEndIndex << "\n"
                    << "The include directive or file is ill-formed.");
                }

                std::string contentBeforeInclude = xmlContent.substr(0, includeStartIndex);
                std::string contentAfterInclude = xmlContent.substr(includeEndIndex, xmlContent.size());

                xmlContent = contentBeforeInclude + includedContent + contentAfterInclude;
            }

            void removeComments(std::string& xmlString)
            {
                std::regex regexp("<!--[\\s\\S\\n]*?-->"); // <!--.--> doesn't work (also, . does not include newlines etc.)
                xmlString = std::regex_replace(xmlString, regexp, ""); 
            }
    };
}