from typing import Type, Dict, List, Union
from google.protobuf.message import Message
from google.protobuf.descriptor import Descriptor
from google.protobuf.message_factory import MessageFactory
from module.type_mapping.mutator import Mutator
from module.type_mapping.proto_codec import ProtoCodec
from dispatch.proto.claidservice_pb2 import DataPackage, NumberMap, StringMap, NumberArray, StringArray, Blob
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
    def get_mutator(example_instance) -> Mutator:
        cls = type(example_instance)
        print(cls)
        if issubclass(cls, int) or issubclass(cls, float):
            return Mutator(
                setter = lambda packet, value: setattr(packet, "number_val", value),
                getter = lambda packet: packet.number_val
            )
        elif cls == bool:
            return Mutator(
                setter = lambda packet, value: setattr(packet, "bool_val", value),
                getter = lambda packet: packet.bool_val
            )
        elif cls == str:
            return Mutator(
                setter = lambda packet, value: setattr(packet, "string_val", value),
                getter = lambda packet: packet.string_val
            )
        elif issubclass(cls, np.ndarray):
            
            if example_instance.dtype in [np.float32, np.float64, \
            np.int8, np.int16, np.int32, np.int64, \
            np.uint8, np.uint16, np.uint32, np.uint64]:
                return Mutator(
                    lambda packet, value: packet.number_array_val.CopyFrom(NumberArray(val=value)),
                    lambda packet: np.array([val for val in packet.number_array_val])
                )
            elif np.issubdtype(example_instance.dtype, np.str_):
                return Mutator(
                    lambda packet, value: packet.string_array_val.CopyFrom(StringArray(val=value)),
                    lambda packet: np.array([val for val in packet.string_array_val])
                )
            else:
                raise Exception("Unsupported type for array or list in TypeMapping. Cannot use type \"{}\" to set or get payload of DataPackage.".format(str(example_instance.dtype)))
        elif issubclass(cls, dict):
            if(len(example_instance) != 1):
                raise Exception("Failed to get Mutator for dict. The dict instance was expected to contain exactly one element, but it contained {}.".format(len(example_instance)))
            
            key_type = list(example_instance.keys())[0]
            value_type = list(example_instance.values())[0]

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
                        lambda packet, value: packet.number_map.CopyFrom(NumberMap(val=value)),
                        lambda packet: dict(packet.number_map.val)
                    )
                elif value_type == str:
                    return Mutator(
                        lambda packet, value: packet.number_map.CopyFrom(StringMap(val=value)),
                        lambda packet: dict(packet.string_map.val)
                    )
                else:
                    raise Exception("Value type {} is not supported for type mapping in CLAID".format(value_type))

            else:
                raise Exception("Unsupported key type for dictionary: {}. Only strings are allowed as keys in type mapping in CLAID".format(key_type))

 
        elif issubclass(cls, Message):
            instance = cls()
            return Mutator(

                lambda packet, value: packet.blob_val.CopyFrom(TypeMapping.get_proto_codec(value).encode(value)),
                lambda packet: TypeMapping.get_proto_codec(instance).decode(packet.blob_val)
            )
        else:
            raise Exception("Unsupported type in TypeMapping. Cannot use type \"{}\" to set or get payload of DataPackage.".format(str(cls)))

    