#pragma once

#include <functional>
#include <type_traits>

#include "dispatch/core/Module/TypeMapping/Mutator.hh"
#include "dispatch/core/Traits/is_integer_no_bool.hh"
#include "dispatch/core/Traits/is_specialization_of.hh"

#include "dispatch/proto/claidservice.grpc.pb.h"
#include "dispatch/core/Module/TypeMapping/ProtoCodec.hh"
#include "dispatch/core/Module/TypeMapping/AnyProtoType.hh"

using claidservice::DataPackage;
using claidservice::NumberMap;
using claidservice::StringMap;
using claidservice::NumberArray;
using claidservice::StringArray;
using claidservice::ControlPackage;

namespace claid {

    class TypeMapping
    {
        static std::map<std::string, ProtoCodec> protoCodecMap;

        static ProtoCodec& getProtoCodec(const google::protobuf::Message* instance) 
        {
            const std::string fullName =  instance->GetDescriptor()->full_name();
            auto it = protoCodecMap.find(fullName);
            if(it == protoCodecMap.end())
            {
                std::shared_ptr<const google::protobuf::Message> msg(instance->New());
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

                    if(packet.payload_oneof_case() != DataPackage::PayloadOneofCase::kBlobVal)
                    {
                        Logger::logError("Invalid package, payload type mismatch! Expected \"%d\" but got \"%d\"", DataPackage::PayloadOneofCase::kBlobVal, packet.payload_oneof_case());
                        throw std::invalid_argument("ProtoCodec.decode failed. Wrong payload type.");
                    }
                    
                    if(!protoCodec.decode(packet.blob_val(), static_cast<google::protobuf::Message*>(&returnValue)))
                    {
                        throw std::invalid_argument("ProtoCodec.decode failed");
                    }
                }
            );
        }

        template <typename T>
        static std::shared_ptr<google::protobuf::Message> convertToMessage(const T& payload) 
        {
            std::shared_ptr<google::protobuf::Message> msg = std::make_shared<T>(payload);
            return msg;
        }

        template<typename T>
        typename std::enable_if<std::is_same<T, AnyProtoType>::value, Mutator<T>>::type
        static getMutator()
        {
            Logger::logInfo("Is AnyProtoType in typemapper");
        
            return Mutator<T>(
                [](DataPackage& packet, const T& value) 
                { 
                    std::shared_ptr<const google::protobuf::Message> message = value.getMessage();
                    if(message == nullptr)
                    {
                        throw std::invalid_argument("Failed to get data of type AnyProtoMessage from DataPacakge. Value of AnyProtoMessage is nullptr");
                    }

                    if (auto controlVal = std::dynamic_pointer_cast<const claidservice::ControlPackage>(message)) 
                    {
                    // Handle ControlVal case
                    packet.set_allocated_control_val(new ControlPackage(*controlVal));
                    std::cout << "Handling ControlVal case" << std::endl;
                    } 
                    // else if (auto numberVal = std::dynamic_pointer_cast<claidservice::NumberVal>(value)) 
                    // {
                    //     packet.set_allocated_number_val(new NumberVal(*numberVal));
                    // } 
                    // else if (auto stringVal = std::dynamic_pointer_cast<claidservice::StringVal>(value)) 
                    // {
                    //     packet.set_allocated_string_val(new StringVal(*stringVal));
                    // } 
                    // else if (auto boolVal = std::dynamic_pointer_cast<claidservice::BoolVal>(value))
                    // {
                    //     packet.set_allocated_bool_val(new BoolVal(*boolVal));
                    // } 
                    else if (auto numberArrayVal = std::dynamic_pointer_cast<const NumberArray>(message)) 
                    {
                        packet.set_allocated_number_array_val(new NumberArray(*numberArrayVal));
                    } 
                    else if (auto stringArrayVal = std::dynamic_pointer_cast<const StringArray>(message)) 
                    {
                        packet.set_allocated_string_array_val(new StringArray(*stringArrayVal));
                    } 
                    else if (auto numberMapVal = std::dynamic_pointer_cast<const NumberMap>(message)) 
                    {
                        packet.set_allocated_number_map(new NumberMap(*numberMapVal));
                    } 
                    else if (auto stringMapVal = std::dynamic_pointer_cast<const StringMap>(message)) 
                    {
                        packet.set_allocated_string_map(new StringMap(*stringMapVal));
                    }
                    else
                    {
                        ProtoCodec& protoCodec = getProtoCodec(message.get());

                        Blob& blob = *packet.mutable_blob_val();

                        if(!protoCodec.encode(value.getMessage().get(), blob))
                        {
                            throw std::invalid_argument("ProtoCodec.encode failed for AnyProtoType");
                        }
                    }

                    

                },
                [](const DataPackage& packet, T& returnValue) 
                { 
                    


                    if(packet.payload_oneof_case() != DataPackage::PayloadOneofCase::kBlobVal)
                    {
                        std::shared_ptr<google::protobuf::Message> message;
                         switch (packet.payload_oneof_case()) 
                         {
                            case DataPackage::kControlVal: {
                                message = convertToMessage(packet.control_val());
                                break;
                            }
                            case DataPackage::kNumberVal: {
                                Logger::logError("Error, number val currently not supported by AnyProtoType.");
                                throw std::invalid_argument("ProtoCodec.decode failed for AnyProtoType, number val currently not supported");
                                //message = convertToMessage(packet.number_val());
                                break;
                            }
                            case DataPackage::kStringVal: {
                                Logger::logError("Error, string val currently not supported by AnyProtoType.");
                                throw std::invalid_argument("ProtoCodec.decode failed for AnyProtoType, string val currently not supported");
                                //message = convertToMessage(packet.string_val());
                                break;
                            }
                            case DataPackage::kBoolVal: {
                                Logger::logError("Error, bool val currently not supported by AnyProtoType.");
                                throw std::invalid_argument("ProtoCodec.decode failed for AnyProtoType, bool val currently not supported");
                                //message = convertToMessage(packet.bool_val());
                                break;
                            }
                            case DataPackage::kNumberArrayVal: {
                                message = convertToMessage(packet.number_array_val());
                                break;
                            }
                            case DataPackage::kStringArrayVal: {
                                message = convertToMessage(packet.string_array_val());
                                break;
                            }
                            case DataPackage::kNumberMap: {
                                message = convertToMessage(packet.number_map());
                                break;
                            }
                            case DataPackage::kStringMap: {
                                message = convertToMessage(packet.string_map());
                                break;
                            }
                            default: {
                                // Handle unknown type or add appropriate error handling
                                break;
                            }
                        }
                        returnValue.setMessage(message);
                        return;
                    }

                    const Blob& blob = packet.blob_val();
                    const std::string messageType = blob.message_type();
                    const google::protobuf::Descriptor* desc =
                        google::protobuf::DescriptorPool::generated_pool()
                            ->FindMessageTypeByName(messageType);

                    if(desc == nullptr)
                    {
                        Logger::logError("Failed to deserialize protobuf message from blob. Cannot find descriptor for message \"%s\"."
                            "This message is unknown to the current instance of CLAID. This might happen, if the message was sent by a remotely "
                            "connected instanceof CLAID, which has implemented a new Protobuf message. Make sure that both, the current and remotely connected instance "
                            "of CLAID know the same protobuf types.", messageType.c_str());
                        throw std::invalid_argument("ProtoCodec.decode failed for AnyProtoType");
                    }

                    const google::protobuf::Message* protoType =
                        google::protobuf::MessageFactory::generated_factory()->GetPrototype(desc);


                    if(desc == nullptr)
                    {
                        Logger::logError("Failed to deserialize protobuf message from blob. Cannot find type for message \"%s\", but could find descriptor. This should not happen.", messageType.c_str());
                        throw std::invalid_argument("ProtoCodec.decode failed for AnyProtoType");
                    }


                    std::shared_ptr<google::protobuf::Message> msg(protoType->New());

                    if(!msg->ParseFromString(packet.blob_val().payload()))
                    {
                        throw std::invalid_argument("ProtoCodec.decode failed for AnyProtoType");
                    }

                    returnValue.setMessage(msg);
                    
                    
                }
            );
        }

        
    };

}