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

#include <functional>
#include <type_traits>

#include "dispatch/core/Module/TypeMapping/Mutator.hh"
#include "dispatch/core/Traits/is_integer_no_bool.hh"
#include "dispatch/core/Traits/is_specialization_of.hh"

#include "dispatch/proto/claidservice.grpc.pb.h"
#include "dispatch/core/Module/TypeMapping/ProtoCodec.hh"
#include "dispatch/core/Module/TypeMapping/AnyProtoType.hh"

using namespace claidservice;

namespace claid {

    class TypeMapping
    {
        static std::map<std::string, ProtoCodec> protoCodecMap;

        template<typename T>
        static std::shared_ptr<const google::protobuf::Message> makeMessage()
        {
            return std::static_pointer_cast<const google::protobuf::Message>(std::make_shared<T>());
        }

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

        template<typename T>
        static bool setProtoPayload(DataPackage& packet, T& protoValue)
        {
            ProtoCodec& protoCodec = getProtoCodec(&protoValue);

            Blob& blob = *packet.mutable_payload();

            return protoCodec.encode(static_cast<const google::protobuf::Message*>(&protoValue), blob);
        }

        template<typename T>
        static bool getProtoPayload(const DataPackage& packet, T& returnValue)
        {
            ProtoCodec& protoCodec = getProtoCodec(&returnValue);

            if(packet.payload().message_type() == "")
            {
                Logger::logError("Invalid package, unknown payload! Expected payload type to be specified in message_type of Blob, but got \"\"");
                throw std::invalid_argument("ProtoCodec.decode failed. Wrong payload type.");
            }
            
            if(!protoCodec.decode(packet.payload(), static_cast<google::protobuf::Message*>(&returnValue)))
            {
                throw std::invalid_argument("ProtoCodec.decode failed");
            }
            return true;
        }
        

    public:

        // Number
        template<typename T>
        typename std::enable_if<std::is_arithmetic<T>::value && !std::is_same<T, bool>::value, Mutator<T>>::type
        static getMutator()
        {
            return Mutator<T>(
                makeMessage<NumberVal>(),
                [](DataPackage& packet, const T& value) 
                { 
                    NumberVal protoVal;
                    protoVal.set_val(static_cast<T>(value));
                    
                    setProtoPayload(packet, protoVal);
                },
                [](const DataPackage& packet, T& returnValue) 
                {
                    NumberVal protoVal;
                    getProtoPayload(packet, protoVal); 

                    returnValue = static_cast<T>(protoVal.val());
                }
            );
        }

        // Bool
        template<typename T>
        typename std::enable_if<std::is_same<T, bool>::value, Mutator<T>>::type
        static getMutator()
        {
            return Mutator<bool>(
                makeMessage<BoolVal>(),
                [](DataPackage& packet, const bool& value) 
                { 
                    BoolVal protoVal;
                    protoVal.set_val(value);

                    setProtoPayload(packet, protoVal);
                },
                [](const DataPackage& packet, T& returnValue) 
                { 
                    BoolVal protoVal;
                    getProtoPayload(packet, protoVal);

                    returnValue = protoVal.val();
                }
            );
        }

        // String
        template<typename T>
        typename std::enable_if<std::is_same<T, std::string>::value, Mutator<T>>::type
        static getMutator()
        {
            return Mutator<std::string>(
                makeMessage<StringVal>(),
                [](DataPackage& packet, const std::string& value) 
                { 
                    StringVal protoVal;
                    protoVal.set_val(value);

                    setProtoPayload(packet, protoVal);
                },
                [](const DataPackage& packet, T& returnValue) 
                { 
                    StringVal protoVal;
                    getProtoPayload(packet, protoVal);

                    returnValue = protoVal.val();               
                }
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
                makeMessage<NumberArray>(),
                [](DataPackage& packet, const T& array) 
                { 
                    NumberArray numberArray;
                    for(auto number : array)
                    {
                        numberArray.add_val(number);
                    }
                    setProtoPayload(packet, numberArray);
                },
                [](const DataPackage& packet, T& returnValue) 
                { 
                    NumberArray numberArray;
                    getProtoPayload(packet, numberArray);

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
                makeMessage<StringArray>(),
                [](DataPackage& packet, const T& array) 
                { 
                    StringArray stringArray;
                    for(const auto& str : array)
                    {
                        stringArray.add_val(str);
                    }
                    setProtoPayload(packet, stringArray);
                },
                [](const DataPackage& packet, T& returnValue) 
                { 
                    StringArray stringArray;
                    getProtoPayload(packet, stringArray);

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
                makeMessage<NumberMap>(),
                [](DataPackage& packet, const T& map) 
                { 
                    NumberMap numberMap;
                    for (const auto& pair : map) 
                    {
                        (*numberMap.mutable_val())[pair.first] = pair.second;
                    }
                    setProtoPayload(packet, numberMap);
                },
                [](const DataPackage& packet, T& returnValue) 
                { 
                    returnValue.clear();
                    NumberMap numberMap;
                    getProtoPayload(packet, numberMap);
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
                makeMessage<StringMap>(),
                [](DataPackage& packet, const T& map) 
                { 
                    StringMap stringMap;
                    for (const auto& pair : map) 
                    {
                        (*stringMap.mutable_val())[pair.first] = pair.second;
                    }
                    setProtoPayload(packet, stringMap);
                },
                [](const DataPackage& packet, T& returnValue) 
                { 
                    returnValue.clear();

                    StringMap stringMap;
                    getProtoPayload(packet, stringMap);

                    for (const auto& pair : stringMap.val()) 
                    {
                        returnValue[pair.first] = pair.second;
                    }
                    
                }
            );
        }

        // // void
        // template<typename T>
        // typename std::enable_if<std::is_same<void, T>::value, Mutator<T>>::type
        // static getMutator()
        // {
        //     return Mutator<T>(
        //         makeMessage<google::protobuf::Empty>(),
        //         [](DataPackage& packet, const T& map) 
        //         { 
                    
        //         },
        //         [](const DataPackage& packet, T& returnValue) 
        //         { 
                    
        //         }
        //     );
        // }

        template<typename T>
        typename std::enable_if<std::is_base_of<google::protobuf::Message, T>::value, Mutator<T>>::type
        static getMutator()
        {
            Logger::logInfo("Is protobuf typ pe in typemapper");
        
            return Mutator<T>(
                makeMessage<T>(),
                [](DataPackage& packet, const T& value) 
                { 
                    setProtoPayload(packet, value);
                },
                [](const DataPackage& packet, T& returnValue) 
                { 
                    getProtoPayload(packet, returnValue);
                }
            );
        }


        template<typename T>
        typename std::enable_if<std::is_same<T, AnyProtoType>::value, Mutator<T>>::type
        static getMutator()
        {
            Logger::logInfo("Is AnyProtoType in typemapper");
        
            return Mutator<T>(
                makeMessage<claidservice::CLAIDANY>(),
                [](DataPackage& packet, const T& value) 
                { 
                    std::shared_ptr<const google::protobuf::Message> message = value.getMessage();
                    if(message == nullptr)
                    {
                        throw std::invalid_argument("Failed to get data of type AnyProtoMessage from DataPacakge. Value of AnyProtoMessage is nullptr");
                    }

                    ProtoCodec& protoCodec = getProtoCodec(message.get());

                    Blob& blob = *packet.mutable_payload();

                    if(!protoCodec.encode(value.getMessage().get(), blob))
                    {
                        throw std::invalid_argument("ProtoCodec.encode failed for AnyProtoType");
                    }

                },
                [](const DataPackage& packet, T& returnValue) 
                { 
                    


                    if(packet.payload().message_type() == "")
                    {
                        Logger::logError("Invalid package, unknown payload! Expected payload type to be specified in message_type of Blob, but got \"\"");
                        throw std::invalid_argument("ProtoCodec.decode failed. Wrong payload type.");
                    }

                    const Blob& blob = packet.payload();
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
                     
                        // const google::protobuf::DescriptorPool* pool = google::protobuf::DescriptorPool::generated_pool();
                        // // Get all file descriptors in the pool
                        // const google::protobuf::DescriptorPool::DescriptorMap& descriptors = pool->internal_generated_pool().
                        // Logger::logError("Registered protobuf classes are: ");
                        // // Iterate through file descriptors
                        // for (const auto& entry : descriptors) {
                        //     const google::protobuf::Descriptor* descriptor = entry.second;
                        //     std::cout << "Full Name: " << descriptor->full_name() << std::endl;
                        // }

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

                    if(!msg->ParseFromString(packet.payload().payload()))
                    {
                        throw std::invalid_argument("ProtoCodec.decode failed for AnyProtoType");
                    }

                    returnValue.setMessage(msg);
                    
                    
                }
            );
        }

        
    };

}