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

from typing import Type
from google.protobuf.message import Message
from dispatch.proto.claidservice_pb2 import DataPackage
from logger.logger import Logger

class Mutator:
    def __init__(self, setter=None, getter=None):
        self.setter = setter
        self.getter = getter

    def set_package_payload(self, packet: DataPackage, value):
        if self.setter:
            self.setter(packet, value)

    def get_package_payload(self, packet: DataPackage):
        if self.getter:
            return self.getter(packet)
        return None
