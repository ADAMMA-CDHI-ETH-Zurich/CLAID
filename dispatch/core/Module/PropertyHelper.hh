#pragma once

#include <map>
#include <vector>
#include <string>

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
        
        template<typename T>
        bool getProperty(const std::string& key, T& value, const T& defaultValue) 
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

    private:
        const std::map<std::string, std::string>& properties;
        std::vector<std::string> unknownProperties;
};

}