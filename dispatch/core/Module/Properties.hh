#pragma once

#include <map>
#include <vector>
#include <string>
#include <regex>

#include "dispatch/proto/claidservice.grpc.pb.h"
#include "dispatch/core/Logger/Logger.hh"
#include "dispatch/core/proto_util.hh"

namespace claid {

class Properties
{
    public:
        Properties(const google::protobuf::Struct& properties) : properties(properties)
        {
            
        }

        template<typename T>
        bool getNumberProperty(const std::string& key, T& value) 
        {
            google::protobuf::Value property;
            
            if(lookupProperty(key, property))
            {
                if(property.kind_case() == google::protobuf::Value::KindCase::kNumberValue)
                {
                    value = static_cast<T>(property.number_value());
                }
                else if(property.kind_case() == google::protobuf::Value::KindCase::kStringValue)
                {
                    std::istringstream iss(property.string_value());
                    iss >> value;
                }
                else
                {
                    return false;
                }
                
                return true;
            } 
            else 
            {
                this->unknownProperties.push_back(key);
                return false;
            }
        }

        template<typename T>
        bool getNumberProperty(const std::string& key, T& value, T& defaultValue) 
        {
            google::protobuf::Value property;
            
            if(lookupProperty(key, property))
            {
                if(property.kind_case() == google::protobuf::Value::KindCase::kNumberValue)
                {
                    value = static_cast<T>(property.number_value());
                }
                else if(property.kind_case() == google::protobuf::Value::KindCase::kStringValue)
                {
                    std::istringstream iss(property.string_value());
                    iss >> value;
                }
                else
                {
                    return false;
                }
                
                return true;
            } 
            else 
            {
                value = defaultValue;
                return false;
            }
        }

        bool getStringProperty(const std::string& key, std::string& value) 
        {
            google::protobuf::Value property;
            
            if(lookupProperty(key, property))
            {
                if(property.kind_case() == google::protobuf::Value::KindCase::kStringValue)
                {
                    value = property.string_value();
                    return true;
                }
                
                return false;
            } 
            else 
            {
                this->unknownProperties.push_back(key);
                return false;
            }
        }

        bool getStringProperty(const std::string& key, std::string& value, const std::string& defaultValue) 
        {
            google::protobuf::Value property;
            
            if(lookupProperty(key, property))
            {
                if(property.kind_case() == google::protobuf::Value::KindCase::kStringValue)
                {
                    value = property.string_value();
                    return true;
                }
                
                return false;
            } 
            else 
            {
                value = defaultValue;
                return false;
            }
        }

        bool getBoolProperty(const std::string& key, bool& value) 
        {
            google::protobuf::Value property;
            
            if(lookupProperty(key, property))
            {
                if(property.kind_case() == google::protobuf::Value::KindCase::kBoolValue)
                {
                    value = property.bool_value();
                    return true;
                }
                else if(property.kind_case() == google::protobuf::Value::KindCase::kStringValue)
                {
                     std::string tmp = property.string_value();
                    for(size_t i = 0; i < tmp.size(); i++)
                    {
                        tmp[i] = std::tolower(tmp[i]);
                    }
                    
                    value = tmp == "true";
                    return true;
                }
                return false;
            } 
            else 
            {
                this->unknownProperties.push_back(key);
                return false;
            }
        }

        bool getBoolProperty(const std::string& key, bool& value, const bool& defaultValue) 
        {
            google::protobuf::Value property;
            
            if(lookupProperty(key, property))
            {
                if(property.kind_case() == google::protobuf::Value::KindCase::kBoolValue)
                {
                    value = property.bool_value();
                    return true;
                }
                else if(property.kind_case() == google::protobuf::Value::KindCase::kStringValue)
                {
                    std::string tmp = property.string_value();
                    for(size_t i = 0; i < tmp.size(); i++)
                    {
                        tmp[i] = std::tolower(tmp[i]);
                    }
                    
                    value = tmp == "true";
                    return true;
                }
                return false;
            } 
            else 
            {
                value = defaultValue;
                return false;
            }
        }

        template<typename T>
        bool getObjectProperty(const std::string& key, T& value)
        {
            static_assert(std::is_base_of<google::protobuf::Message, T>::value, "Cannot use class as object property, getObjectProperty only supports protobuf classes."); 
            google::protobuf::Value property;
            
            if(lookupProperty(key, property))
            {
                if(property.kind_case() == google::protobuf::Value::KindCase::kStructValue)
                {

                    // Serialize property to json, and then deserialize into the target proto message object.
                    std::string jsonOutput;
                    google::protobuf::util::JsonPrintOptions options;
                    options.add_whitespace = true;
                    options.always_print_primitive_fields = true;
                    options.preserve_proto_field_names = true;
                    absl::Status status = MessageToJsonString(property, &jsonOutput, options);
                    if(!status.ok())
                    {
                        Logger::logError("Failed get object property: Failed to serialize property \"%s\" to json: %s", key.c_str(), status.ToString().c_str());
                        return false;
                    }

                    google::protobuf::util::JsonParseOptions options2;
                    status = JsonStringToMessage(jsonOutput, &value, options2);

                    if(!status.ok())
                    {
                        Logger::logError("Failed get object property: Failed to deserialize \"%s\" from json: %s", key.c_str(), status.ToString().c_str());
                        return false;
                    }
                    return true;
                }
                
                return false;
            } 
            else 
            {
                this->unknownProperties.push_back(key);
                return false;
            }
        }
        
        
        // template<typename T, typename U>
        // bool getOptionalProperty(const std::string& key, T& value, const U& defaultValue) 
        // {
        //     auto it = properties.find(key);
        //     if (it != properties.end()) 
        //     {
        //         std::istringstream iss(it->second);
        //         iss >> value;
        //         return true;
        //     } 
        //     else 
        //     {
        //         value = defaultValue;
        //         return true;
        //     }
        // }

        // template<typename T>
        // bool getPropertiesWithNameThatMatchesRegularExpression(const std::string& regex, std::vector<T>& values)
        // {
        //     values.clear(); 
        //     std::regex pattern(regex);



        //     for(const auto& entry : properties)
        //     {
        //         if (std::regex_match(entry.first, pattern)) 
        //         {
        //             T tmpValue;
        //             std::istringstream iss(entry.second);
        //             iss >> tmpValue;
        //             values.push_back(tmpValue);
        //         }
        //     }
        //     return values.size() > 0;
        // }

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
        const google::protobuf::Struct properties;
        std::vector<std::string> unknownProperties;

        bool lookupProperty(const std::string& key, google::protobuf::Value& property)
        {
            auto it = properties.fields().find(key);
            if (it != properties.fields().end()) 
            {
                property = it->second;
                return true;
            } 
            else 
            {
                return false;
            }
        }
};

}