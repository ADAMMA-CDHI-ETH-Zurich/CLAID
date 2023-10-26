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
        template<typename T>
        typename std::enable_if<is_integer_no_bool<T>::value, Mutator<T>>::type
        static getMutator()
        {
            return Mutator<T>(
                [](DataPackage& packet, const T& value) { packet.set_number_val(value); },
                [](const DataPackage& packet) { return packet.number_val(); }
            );
        }


        template<typename T>
        typename std::enable_if<std::is_same<T, bool>::value, Mutator<T>>::type
        static getMutator()
        {
            return Mutator<std::string>(
                [](DataPackage& packet, const bool& value) { packet.set_bool_val(value); },
                [](const DataPackage& packet) { return packet.bool_val(); }
            );
        }

        template<typename T>
        typename std::enable_if<std::is_same<T, std::string>::value, Mutator<T>>::type
        static getMutator()
        {
            return Mutator<std::string>(
                [](DataPackage& packet, const std::string& value) { packet.set_string_val(value); },
                [](const DataPackage& packet) { return packet.string_val(); }
            );
        }

        template<typename T>
        typename std::enable_if<is_specialization_of<T, std::vector>::value && is_integer_no_bool<typename T::value_type>::value, Mutator<T>>::type
        static getMutator()
        {
            return Mutator<std::string>(
                [](DataPackage& packet, const T& array) 
                { 
                    NumberArray* numberArray = packet.mutable_number_array_val();
                    for(auto number : array)
                    {
                        numberArray->add_val(number);
                    }
                },
                [](const DataPackage& packet) { return packet.number_array_val(); }
            );
        }
    };

}