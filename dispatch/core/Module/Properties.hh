/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Stephan Altmüller
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*         http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
***************************************************************************/

#pragma once

#include <map>
#include <vector>
#include <string>
#include <regex>

#include "dispatch/proto/claidservice.grpc.pb.h"
#include "dispatch/core/Logger/Logger.hh"
#include "dispatch/core/proto_util.hh"
#include "google/protobuf/json/json.h"

namespace claid {

class Properties
{
    public:
        Properties(const google::protobuf::Struct& properties) : properties(properties)
        {
            
        }

        template<typename T>
        void getNumberProperty(const std::string& key, T& value)
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
                    Logger::throwLogFatalIfNotCaught("Invalid property type! Found property \"%s\", but it's type is invalid (expected number or string).", key.c_str());
                }
            } 
            else 
            {
                this->unknownProperties.push_back(key);
            }
        }

        template<typename T>
        void getNumberProperty(const std::string& key, T& value, const T& defaultValue) 
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
                    Logger::throwLogFatalIfNotCaught("Invalid property type! Found property \"%s\", but it's type is invalid (expected number or string).", key.c_str());
                }
            } 
            else 
            {
                value = defaultValue;
            }
        }

        void getStringProperty(const std::string& key, std::string& value) 
        {
            google::protobuf::Value property;
            
            if(lookupProperty(key, property))
            {
                if(property.kind_case() == google::protobuf::Value::KindCase::kStringValue)
                {
                    value = property.string_value();
                }
                else
                {
                    Logger::throwLogFatalIfNotCaught("Invalid property type! Found property \"%s\", but it's type is invalid (expected string).", key.c_str());            
                }        
            } 
            else 
            {
                this->unknownProperties.push_back(key);
            }
        }

        void getStringProperty(const std::string& key, std::string& value, const std::string& defaultValue) 
        {
            google::protobuf::Value property;
            
            if(lookupProperty(key, property))
            {
                if(property.kind_case() == google::protobuf::Value::KindCase::kStringValue)
                {
                    value = property.string_value();
                }
                else
                {
                    Logger::throwLogFatalIfNotCaught("Invalid property type! Found property \"%s\", but it's type is invalid (expected string).", key.c_str());            
                }  
            } 
            else 
            {
                value = defaultValue;
            }
        }

        void getBoolProperty(const std::string& key, bool& value) 
        {
            google::protobuf::Value property;
            
            if(lookupProperty(key, property))
            {
                if(property.kind_case() == google::protobuf::Value::KindCase::kBoolValue)
                {
                    value = property.bool_value();
                }
                else if(property.kind_case() == google::protobuf::Value::KindCase::kStringValue)
                {
                     std::string tmp = property.string_value();
                    for(size_t i = 0; i < tmp.size(); i++)
                    {
                        tmp[i] = std::tolower(tmp[i]);
                    }
                    
                    value = tmp == "true";
                }
                else
                {
                    Logger::throwLogFatalIfNotCaught("Invalid property type! Found property \"%s\", but it's type is invalid (expected bool or string).", key.c_str());            
                }
            } 
            else 
            {
                this->unknownProperties.push_back(key);
            }
        }

        void getBoolProperty(const std::string& key, bool& value, const bool& defaultValue) 
        {
            google::protobuf::Value property;
            
            if(lookupProperty(key, property))
            {
                if(property.kind_case() == google::protobuf::Value::KindCase::kBoolValue)
                {
                    value = property.bool_value();
                }
                else if(property.kind_case() == google::protobuf::Value::KindCase::kStringValue)
                {
                    std::string tmp = property.string_value();
                    for(size_t i = 0; i < tmp.size(); i++)
                    {
                        tmp[i] = std::tolower(tmp[i]);
                    }
                    
                    value = tmp == "true";
                }
                else
                {
                    Logger::throwLogFatalIfNotCaught("Invalid property type! Found property \"%s\", but it's type is invalid (expected bool or string).", key.c_str());            
                }
            } 
            else 
            {
                value = defaultValue;
            }
        }

        template<typename T>
        void getObjectProperty(const std::string& key, T& value)
        {
            static_assert(std::is_base_of<google::protobuf::Message, T>::value, "Cannot use class as object property, getObjectProperty only supports protobuf classes."); 
            google::protobuf::Value property;
            
            if(lookupProperty(key, property))
            {
                if(property.kind_case() == google::protobuf::Value::KindCase::kStructValue)
                {
                    // Serialize property to json, and then deserialize into the target proto message object.
                    std::string jsonOutput;
                    google::protobuf::json::PrintOptions options;
                    options.add_whitespace = true;
                    options.always_print_fields_with_no_presence = true;
                    options.preserve_proto_field_names = true;
                    absl::Status status = MessageToJsonString(property, &jsonOutput, options);
                    if(!status.ok())
                    {
                        Logger::throwLogFatalIfNotCaught("Failed to get object property: Failed to serialize property \"%s\" to json: %s", key.c_str(), status.ToString().c_str());
                        return;
                    }

                    google::protobuf::util::JsonParseOptions options2;
                    status = JsonStringToMessage(jsonOutput, &value, options2);

                    if(!status.ok())
                    {
                        Logger::throwLogFatalIfNotCaught("Failed to get object property: Failed to deserialize \"%s\" from json: %s", key.c_str(), status.ToString().c_str());
                        return;
                    }
                }
                else
                {
                    Logger::throwLogFatalIfNotCaught("Invalid property type! Found property \"%s\", but it's type is invalid (expected object type).", key.c_str());
                }
            } 
            else 
            {
                this->unknownProperties.push_back(key);
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