###########################################################################
# Copyright (C) 2023 ETH Zurich
# CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
# Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
# Centre for Digital Health Interventions (c4dhi.org)
# 
# Authors: Patrick Langer
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#         http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################

from typing import Type, Dict, List, Union
from google.protobuf.message import Message
from google.protobuf.descriptor import Descriptor
from google.protobuf.message_factory import MessageFactory
from module.type_mapping.mutator import Mutator
from module.type_mapping.proto_codec import ProtoCodec
from dispatch.proto.claidservice_pb2 import DataPackage, IntVal, DoubleVal, BoolVal, StringVal, NumberMap, StringMap, NumberArray, StringArray, Blob
import numpy as np

class TypeMapping:
    protoCodecMap: Dict[str, ProtoCodec] = {}

    @staticmethod
    def get_proto_codec(instance: Message) -> ProtoCodec:
        full_name = instance.DESCRIPTOR.full_name
        if full_name not in TypeMapping.protoCodecMap:
            msg = instance.__class__()
            codec = ProtoCodec(msg)
            TypeMapping.protoCodecMap[full_name] = codec
        return TypeMapping.protoCodecMap[full_name]

    @staticmethod
    def setProtoPayload(packet: DataPackage, value: Message):

        proto_codec = TypeMapping.get_proto_codec(value)

        blob = proto_codec.encode(value)
        packet.payload.CopyFrom(blob)

        print("Test package", packet)
        print("Value", value)
        return packet
    
    @staticmethod
    def getProtoPayload(packet: DataPackage, example_instance: Message):
        
        proto_codec = TypeMapping.get_proto_codec(example_instance)
        data = proto_codec.decode(packet.payload)

        return data


    @staticmethod
    def get_mutator(example_instance) -> Mutator:
        cls = type(example_instance)
        print("Clz ", cls)
        if cls == int:
            
            return Mutator(
                setter = lambda packet, value: TypeMapping.setProtoPayload(packet, IntVal(val=value)),
                getter = lambda packet: TypeMapping.getProtoPayload(packet, IntVal()).val
            )
        
        # Actually corresponds to double in C++ -> 64 bit floating point, no separate double type.
        if cls == float:
            
            return Mutator(
                setter = lambda packet, value: TypeMapping.setProtoPayload(packet, DoubleVal(val=value)),
                getter = lambda packet: TypeMapping.getProtoPayload(packet, DoubleVal()).val
            )
        
        elif cls == bool:
            return Mutator(
                setter = lambda packet, value: TypeMapping.setProtoPayload(packet, BoolVal(val=value)),
                getter = lambda packet: TypeMapping.getProtoPayload(packet, BoolVal()).val
            )
        elif cls == str:
            return Mutator(
                setter = lambda packet, value: TypeMapping.setProtoPayload(packet, StringVal(val=value)),
                getter = lambda packet: TypeMapping.getProtoPayload(packet, StringVal()).val
            )
        elif issubclass(cls, np.ndarray):
            
            if example_instance.dtype in [np.float32, np.float64, \
            np.int8, np.int16, np.int32, np.int64, \
            np.uint8, np.uint16, np.uint32, np.uint64]:
                a = Mutator(
                    lambda packet, value: TypeMapping.setProtoPayload(packet, NumberArray(val=value)),
                    lambda packet: np.array([val for val in TypeMapping.getProtoPayload(packet, NumberArray).val])
                )
                print("Made mutator, returning")
                return a
            elif np.issubdtype(example_instance.dtype, np.str_):
                return Mutator(
                    lambda packet, value: TypeMapping.setProtoPayload(packet, StringArray(val=value)),
                    lambda packet: np.array([val for val in TypeMapping.getProtoPayload(packet, StringArray).val])
                )
            else:
                raise Exception("Unsupported type for array or list in TypeMapping. Cannot use type \"{}\" to set or get payload of DataPackage.".format(str(example_instance.dtype)))
        elif issubclass(cls, dict):
            if(len(example_instance) != 1):
                raise Exception("Failed to get Mutator for dict. The dict instance was expected to contain exactly one element, but it contained {}.".format(len(example_instance)))
            
            key_type = type(list(example_instance.keys())[0])
            value_type = type(list(example_instance.values())[0])

            print(key_type, type(key_type), isinstance(type, type(key_type)))
            if(not isinstance(type, type(key_type))):
                raise Exception("Unsupported dictionary for type mapping. Expected key to be a type, but got: {}.\n"
                        "If you want to use a dictionary, use types as keys and values, e.g.: {{str: int}} "
                        "(to make a dictionary containing <string, int> pairs).".format(key_type))


            if(not isinstance(type, type(value_type))):
                raise Exception("Unsupported dictionary for type mapping. Expected value to be a type, but got: {}\n"
                "If you want to use a dictionary, use types as keys and values, e.g.: {{str: int}} "
                "(to make a dictionary containing <string, int> pairs).".format(value_type))


            if key_type == str:
                
                if value_type in [int, float]:
                    return Mutator(
                        lambda packet, value: TypeMapping.setProtoPayload(packet, NumberMap(val=value)),
                        lambda packet: dict(TypeMapping.getProtoPayload(packet, NumberMap()).val)
                    )
                elif value_type == str:
                    return Mutator(
                        lambda packet, value: (
                            print(value),
                            TypeMapping.setProtoPayload(packet, StringMap(val=value))
                        ),
                        lambda packet: dict(TypeMapping.getProtoPayload(packet, StringMap).val)
                    )
                else:
                    raise Exception("Value type {} is not supported for type mapping in CLAID".format(value_type))

            else:
                raise Exception("Unsupported key type for dictionary: {}. Only strings are allowed as keys in type mapping in CLAID".format(key_type))

 
        elif issubclass(cls, Message):
            instance = cls()
            return Mutator(

                lambda packet, value: packet.payload.CopyFrom(TypeMapping.get_proto_codec(value).encode(value)),
                lambda packet: TypeMapping.get_proto_codec(instance).decode(packet.payload)
            )
        else:
            raise Exception("Unsupported type in TypeMapping. Cannot use type \"{}\" to set or get payload of DataPackage.".format(str(cls)))

    