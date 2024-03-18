#pragma once

#include <map>
#include <vector>
#include <string>
#include <regex>

namespace claid {

class PropertyHelper
{
    public:
        PropertyHelper(const std::map<std::string, std::string>& properties) : properties(properties)
        {
            
        }


        template<typename T>
        bool getProperty(const std::string& key, T& value) 
        {
            auto it = properties.find(key);
            if (it != properties.end()) 
            {
                std::istringstream iss(it->second);
                iss >> value;
                return true;
            } 
            else 
            {
                this->unknownProperties.push_back(key);
                return false;
            }
        }
        
        template<typename T, typename U>
        bool getOptionalProperty(const std::string& key, T& value, const U& defaultValue) 
        {
            auto it = properties.find(key);
            if (it != properties.end()) 
            {
                std::istringstream iss(it->second);
                iss >> value;
                return true;
            } 
            else 
            {
                value = defaultValue;
                return true;
            }
        }

        template<typename T>
        bool getPropertiesWithNameThatMatchesRegularExpression(const std::string& regex, std::vector<T>& values)
        {
            values.clear(); 
            std::regex pattern(regex);

            for(const auto& entry : properties)
            {
                if (std::regex_match(entry.first, pattern)) 
                {
                    T tmpValue;
                    std::istringstream iss(entry.second);
                    iss >> tmpValue;
                    values.push_back(tmpValue);
                }
            }
            return values.size() > 0;
        }

        bool wasAnyPropertyUnknown() const
        {
            return this->unknownProperties.size() > 0;
        }

        const std::vector<std::string>& getUnknownProperties() const
        {
            return this->unknownProperties;
        }

        void unknownPropertiesToString(std::string& string) const 
        {
            for(const std::string& property : this->unknownProperties)
            {
                if(string == "")
                {
                    string = property;
                }
                else
                {
                    string += std::string(", ") + property;
                }
            }
        }

        std::string getMissingPropertiesErrorString()
        {
            std::string unknownProperties;
            unknownPropertiesToString(unknownProperties);
            return "Missing properties: [" + unknownProperties +  "]. Please sepcify the properties in the configuration file.";
        }


    private:
        const std::map<std::string, std::string>& properties;
        std::vector<std::string> unknownProperties;
};

}