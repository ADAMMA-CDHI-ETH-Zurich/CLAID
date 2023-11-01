#pragma once

#include <functional>
#include <type_traits>

#include "dispatch/core/Module/TypeMapping/Mutator.hh"
#include "dispatch/core/Traits/is_integer_no_bool.hh"
#include "dispatch/core/Traits/is_specialization_of.hh"

#include "dispatch/proto/claidservice.grpc.pb.h"

using claidservice::DataPackage;
using claidservice::NumberMap;
using claidservice::StringMap;
using claidservice::NumberArray;
using claidservice::StringArray;

namespace claid {

    class TypeMapping
    {
    public:

        // Number
        template<typename T>
        typename std::enable_if<std::is_arithmetic<T>::value && !std::is_same<T, bool>::value, Mutator<T>>::type
        static getMutator()
        {
            return Mutator<T>(
                [](DataPackage& packet, const T& value) { packet.set_number_val(value); },
                [](const DataPackage& packet) { return packet.number_val(); }
            );
        }

        // Bool
        template<typename T>
        typename std::enable_if<std::is_same<T, bool>::value, Mutator<T>>::type
        static getMutator()
        {
            return Mutator<std::string>(
                [](DataPackage& packet, const bool& value) { packet.set_bool_val(value); },
                [](const DataPackage& packet) { return packet.bool_val(); }
            );
        }

        // String
        template<typename T>
        typename std::enable_if<std::is_same<T, std::string>::value, Mutator<T>>::type
        static getMutator()
        {
            return Mutator<std::string>(
                [](DataPackage& packet, const std::string& value) { packet.set_string_val(value); },
                [](const DataPackage& packet) { return packet.string_val(); }
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
                [](const DataPackage& packet) 
                { 
                    const NumberArray& numberArray = packet.number_array_val();

                    T array(numberArray.val_size());
                    for (int i = 0; i < numberArray.val_size(); i++) 
                    {
                        array[i] = numberArray.val(i);
                    }

                    return array;
                    
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
                [](const DataPackage& packet) 
                { 
                    const StringArray& stringArray = packet.string_array_val();

                    T array(stringArray.val_size());
                    for (int i = 0; i < stringArray.val_size(); i++) 
                    {
                        array[i] = stringArray.val(i);
                    }

                    return array;
                    
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
                [](const DataPackage& packet) 
                { 
                    T result;
                    const NumberMap& numberMap = packet.number_map();
                    for (const auto& pair : numberMap.val()) 
                    {
                        result[pair.first] = pair.second;
                    }
                    
                    return result;
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
                [](const DataPackage& packet) 
                { 
                    T result;
                    const StringMap& stringMap = packet.string_map();
                    for (const auto& pair : stringMap.val()) 
                    {
                        result[pair.first] = pair.second;
                    }
                    
                    return result;
                }
            );
        }
    };

}