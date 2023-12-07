#pragma once

#include <functional>
#include <type_traits>

#include "dispatch/core/Module/TypeMapping/Mutator.hh"
#include "dispatch/core/Traits/is_integer_no_bool.hh"
#include "dispatch/core/Traits/is_specialization_of.hh"

#include "dispatch/proto/claidservice.grpc.pb.h"
#include "dispatch/core/Module/TypeMapping/ProtoCodec.hh"


using claidservice::DataPackage;
using claidservice::NumberMap;
using claidservice::StringMap;
using claidservice::NumberArray;
using claidservice::StringArray;

namespace claid {

    class TypeMapping
    {
        static std::map<std::string, ProtoCodec> protoCodecMap;

        template<typename T>
        static ProtoCodec& getProtoCodec(const T* instance) 
        {
           

            const std::string fullName =  instance->GetDescriptor()->full_name();
            auto it = protoCodecMap.find(fullName);
            if(it == protoCodecMap.end())
            {
                std::shared_ptr<T> instance = std::make_shared<T>();

                std::shared_ptr<const google::protobuf::Message> msg 
                    = std::static_pointer_cast<const google::protobuf::Message>(instance);

                ProtoCodec codec(msg);
                protoCodecMap.insert(make_pair(fullName, codec));
                return protoCodecMap[fullName];
            }

            return it->second;
        }
        

    public:

        // Number
        template<typename T>
        typename std::enable_if<std::is_arithmetic<T>::value && !std::is_same<T, bool>::value, Mutator<T>>::type
        static getMutator()
        {
            return Mutator<T>(
                [](DataPackage& packet, const T& value) { packet.set_number_val(value); },
                [](const DataPackage& packet, T& returnValue) { returnValue = packet.number_val(); }
            );
        }

        // Bool
        template<typename T>
        typename std::enable_if<std::is_same<T, bool>::value, Mutator<T>>::type
        static getMutator()
        {
            return Mutator<bool>(
                [](DataPackage& packet, const bool& value) { packet.set_bool_val(value); },
                [](const DataPackage& packet, T& returnValue) { returnValue = packet.bool_val(); }
            );
        }

        // String
        template<typename T>
        typename std::enable_if<std::is_same<T, std::string>::value, Mutator<T>>::type
        static getMutator()
        {
            return Mutator<std::string>(
                [](DataPackage& packet, const std::string& value) { packet.set_string_val(value); },
                [](const DataPackage& packet, T& returnValue) { returnValue = packet.string_val(); }
            );
        }

        // NumberArray
        template<typename T>
        typename std::enable_if<is_specialization_of<T, std::vector>::value && 
            std::is_arithmetic<typename T::value_type>::value && 
            !std::is_same<typename T::value_type, bool>::value, Mutator<T>>::type
        static getMutator()
        {
            return Mutator<T>(
                [](DataPackage& packet, const T& array) 
                { 
                    NumberArray* numberArray = packet.mutable_number_array_val();
                    for(auto number : array)
                    {
                        numberArray->add_val(number);
                    }
                },
                [](const DataPackage& packet, T& returnValue) 
                { 
                    const NumberArray& numberArray = packet.number_array_val();

                    returnValue = T(numberArray.val_size());
                    for (int i = 0; i < numberArray.val_size(); i++) 
                    {
                        returnValue[i] = numberArray.val(i);
                    }

                    
                }
            );
        }

        // StringArray
        template<typename T>
        typename std::enable_if<is_specialization_of<T, std::vector>::value && std::is_same<std::string, typename T::value_type>::value, Mutator<T>>::type
        static getMutator()
        {
            return Mutator<T>(
                [](DataPackage& packet, const T& array) 
                { 
                    StringArray* stringArray = packet.mutable_string_array_val();
                    for(const auto& str : array)
                    {
                        stringArray->add_val(str);
                    }
                },
                [](const DataPackage& packet, T& returnValue) 
                { 
                    const StringArray& stringArray = packet.string_array_val();

                    returnValue = T(stringArray.val_size());
                    for (int i = 0; i < stringArray.val_size(); i++) 
                    {
                        returnValue[i] = stringArray.val(i);
                    }                   
                }
            );
        }

        // NumberMap
        template<typename T>
        typename std::enable_if<is_specialization_of<T, std::map>::value && 
            std::is_arithmetic<typename T::mapped_type>::value && 
            std::is_same<typename T::key_type, std::string>::value &&
            !std::is_same<typename T::mapped_type, bool>::value, Mutator<T>>::type
        static getMutator()
        {
            return Mutator<T>(
                [](DataPackage& packet, const T& map) 
                { 
                    NumberMap* numberMap = packet.mutable_number_map();
                    for (const auto& pair : map) 
                    {
                        (*numberMap->mutable_val())[pair.first] = pair.second;
                    }
                },
                [](const DataPackage& packet, T& returnValue) 
                { 
                    returnValue.clear();
                    const NumberMap& numberMap = packet.number_map();
                    for (const auto& pair : numberMap.val()) 
                    {
                        returnValue[pair.first] = pair.second;
                    }
                }
            );
        }

        // StringMap
        template<typename T>
        typename std::enable_if<std::is_same<std::map<std::string, std::string>, T>::value, Mutator<T>>::type
        static getMutator()
        {
            return Mutator<T>(
                [](DataPackage& packet, const T& map) 
                { 
                    StringMap* stringMap = packet.mutable_string_map();
                    for (const auto& pair : map) 
                    {
                        (*stringMap->mutable_val())[pair.first] = pair.second;
                    }
                },
                [](const DataPackage& packet, T& returnValue) 
                { 
                    returnValue.clear();

                    const StringMap& stringMap = packet.string_map();
                    for (const auto& pair : stringMap.val()) 
                    {
                        returnValue[pair.first] = pair.second;
                    }
                    
                }
            );
        }

        template<typename T>
        typename std::enable_if<std::is_base_of<google::protobuf::Message, T>::value, Mutator<T>>::type
        static getMutator()
        {
            Logger::logInfo("Is protobuf typ pe in typemapper");
        
            return Mutator<T>(
                [](DataPackage& packet, const T& value) 
                { 

                    ProtoCodec& protoCodec = getProtoCodec(&value);

                    Blob& blob = *packet.mutable_blob_val();

                    protoCodec.encode(static_cast<const google::protobuf::Message*>(&value), blob);

                },
                [](const DataPackage& packet, T& returnValue) 
                { 
                    ProtoCodec& protoCodec = getProtoCodec(&returnValue);
                    
                    if(!protoCodec.decode(packet.blob_val(), static_cast<google::protobuf::Message*>(&returnValue)))
                    {
                        throw std::invalid_argument("ProtoCodec.decode failed");
                    }
                }
            );
        }
    };

}