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
            
            # In Python, protobuf might decide to leave the "payload" field of the blob as None, if a
            # certain message only has default values. For example, if the payload is a NumberVal, and NumberVal.val = 0, then
            # blob.payload would be None (or 0), because protobuf considers NumberVal.val = 0 to be the default case and hence does not serialize any data.
            # In such case, we simply return a default value.
            if len(blob.payload) == 0:
                Logger.log_warning("Cannot parse protobuf message of type {}. Payload data is null. Very likely the sender Module postet a protobuf message without data".format(self.full_name))
            
                return return_value

            else:
                if not return_value.ParseFromString(blob.payload):
                    Logger.log_error("Failed to parse protobuf message of type {}. ParseFromString failed.").format(self.full_name)
                
                    return None

            return return_value
        except Exception as e:
            # Replace with actual error handling/logging
            Logger.log_error("Failed to parse protobuf message from Blob: {}".format(str(e)))
            return None
