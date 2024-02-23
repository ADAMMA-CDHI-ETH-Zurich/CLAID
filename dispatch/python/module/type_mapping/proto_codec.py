import logging
from typing import Optional
from google.protobuf.message import Message
from google.protobuf.any_pb2 import Any
from dispatch.proto.claidservice_pb2 import Blob, Codec

from logger.logger import Logger


class ProtoCodec:
    def __init__(self, message):
        self.full_name = message.DESCRIPTOR.full_name 
        self.msg = message

    def encode(self, proto_message: Message) -> Blob:

        return_blob = Blob()

        name = proto_message.DESCRIPTOR.full_name
        if name != self.full_name:
            return None

        serialized_data = proto_message.SerializeToString()

        return_blob.codec = Codec.CODEC_PROTO
        return_blob.payload = serialized_data
        return_blob.message_type = self.full_name

        return return_blob

    def decode(self, blob: Blob) -> Message:
        return_value = self.msg.__class__()

        try:
            
            if len(blob.payload) == 0:
                Logger.log_warning("Cannot parse protobuf message of type {}. Payload data is null. Very likely the sender Module postet a protobuf message without data").format(self.full_name)
                return return_value

            if not return_value.ParseFromString(blob.payload):
                Logger.log_error("Failed to parse protobuf message of type {}. ParseFromString failed.").format(self.full_name)
            
                return None

            return return_value
        except Exception as e:
            # Replace with actual error handling/logging
            Logger.log_error("Failed to parse protobuf message from Blob: %s", str(e))
            return None
