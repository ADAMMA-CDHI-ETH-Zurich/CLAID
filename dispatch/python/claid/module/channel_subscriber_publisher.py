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

from typing import Type, TypeVar, List, Dict, Union
from queue import SimpleQueue
from module.abstract_subscriber import AbstractSubscriber
from module.type_mapping.type_mapping import TypeMapping
from module.type_mapping.mutator import Mutator
from module.channel import Channel
from module.publisher import Publisher

from dispatch.proto.claidservice_pb2 import DataPackage

from dispatch.proto.claidservice_pb2 import * 

# Assuming you have a Python protobuf equivalent for claidservice::DataPackage
# from your_protobuf_module import DataPackage



class ChannelSubscriberPublisher:
    def __init__(self, to_module_manager_queue):
        self.__example_packages_for_each_module: Dict[str, List[DataPackage]] = {}
        self.__module_channels_subscriber_map: Dict[tuple, List[AbstractSubscriber]] = {}
        self.__to_module_manager_queue = to_module_manager_queue

    def prepare_example_package(self, data_type_example, module_id: str, channel_name: str, is_publisher: bool) -> DataPackage:
        data_package = DataPackage()

        if is_publisher:
            data_package.source_module = module_id
        else:
            data_package.target_module = module_id

        print(type(channel_name))
        data_package.channel = channel_name

        print("Preparing example package for channel ", channel_name, type(data_type_example))

        mutator = TypeMapping.get_mutator(example_instance=data_type_example)

        mutator.set_package_payload(data_package, data_type_example)
        return data_package

    def publish(self, data_type_example, module, channel_name: str) -> Channel:
        module_id = module.get_id()
        example_package = self.prepare_example_package(data_type_example, module_id, channel_name, True)

        print(f"Inserting package for Module {module_id}")
        self.__example_packages_for_each_module.setdefault(module_id, []).append(example_package)

        publisher = Publisher(data_type_example, module_id, channel_name, self.__to_module_manager_queue)
        return Channel(module, channel_name, publisher)

    def subscribe(self, data_type_example, module, channel_name: str, subscriber: AbstractSubscriber) -> Channel:
        module_id = module.get_id()
        example_package = self.prepare_example_package(data_type_example, module_id, channel_name, False)

        # setdefault() method returns the value of the item with the specified key.
        # If the key does not exist, insert the key, with the specified value, see example below
        self.__example_packages_for_each_module.setdefault(module_id, []).append(example_package)
        self.insert_subscriber(channel_name, module_id, subscriber)

        return Channel(module, channel_name, subscriber)

    def insert_subscriber(self, channel_name: str, module_id: str, subscriber: AbstractSubscriber):
        channel_module_key = (channel_name, module_id)

        if channel_module_key not in self.__module_channels_subscriber_map:
            self.__module_channels_subscriber_map[channel_module_key] = []

        self.__module_channels_subscriber_map[channel_module_key].append(subscriber)

    def get_subscriber_instances_of_module(self, channel_name: str, module_id: str) -> List[AbstractSubscriber]:
        channel_module_key = (channel_name, module_id)

        return self.__module_channels_subscriber_map.get(channel_module_key, [])

    def get_channel_template_packages_for_module(self, module_id: str) -> List[DataPackage]:
        return self.__example_packages_for_each_module.get(module_id, [])

    def is_data_package_compatible_with_channel(self, data_package: DataPackage, receiver_module: str) -> bool:
        channel_name = data_package.channel

        if receiver_module in self.__example_packages_for_each_module:
            for template_package in self.__example_packages_for_each_module[receiver_module]:
                if template_package.channel == channel_name:
                    return template_package.payload.message_type == data_package.payload.message_type

        return False

    def get_payload_case_of_channel(self, channel_name: str, receiver_module: str) -> str:
        if receiver_module in self.__example_packages_for_each_module:
            print(self.__example_packages_for_each_module)
            for template_package in self.__example_packages_for_each_module[receiver_module]:
                if template_package.channel == channel_name:
                    return template_package.payload.message_type

        return ""
    
    def reset(self):
        self.__example_packages_for_each_module = {}
        self.__module_channels_subscriber_map = {}

