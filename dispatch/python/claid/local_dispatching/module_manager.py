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

from module.module_factory import ModuleFactory
from logger.logger import Logger
from module.channel_subscriber_publisher import ChannelSubscriberPublisher
from module.module_annotator import ModuleAnnotator
from module.thread_safe_channel import ThreadSafeChannel
from dispatch.proto.claidservice_pb2 import *
import traceback
from threading import Thread
import time

import sys

class ModuleManager():

    def __init__(self, module_dispatcher, module_factory, main_thread_runnables_queue):
        self.__module_dispatcher = module_dispatcher
        self.__module_factory = module_factory
        # In Python, it is possible to add Modules at runtime.
        self.main_thread_runnables_queue = main_thread_runnables_queue

       
        self.__running_modules = dict()    

        # self.__from_module_dispatcher_queue = ThreadSafeChannel()
        self.__to_module_dispatcher_queue = module_dispatcher.get_to_dispatcher_queue()
        
        self.__channel_subscriber_publisher = ChannelSubscriberPublisher(self.__to_module_dispatcher_queue)

        self.__running = False
        self.__from_module_dispatcher_queue_closed = False

        self.__module__anotations_updated = False

        self.__module_annotations_for_host = dict() # a dict containing the Host name as key and the ModuleAnnotations for the host as value
        self.__module__anotations_updated_for_host = dict() # a dict containg the Host name as key and a boolean value indicating whether the known module annotations are corrected as the value

        self.__on_connected_to_remote_server_callbacks = list()
        self.__on_disconnected_from_remote_server_callbacks = list()

        self.__log_sink_log_message_callback = None

    def instantiate_module(self, module_id, module_class):

        if not self.__module_factory.is_module_class_registered(module_class):
            Logger.log_error(f"ModuleManager: Failed to instantiate Module of class \"{module_class}\" (id: {module_id}\")\n"
                  f"A Module with this class is not registered to the ModuleFactory.")
            return False

        Logger.log_info(f"Loaded Module id \"{module_id}\" (class: \"{module_class}\").")
        module = self.__module_factory.get_instance(module_class, module_id)
        self.__running_modules[module_id] = module
        Logger.log_info(str(self.__running_modules))
        return True

    def instantiate_modules(self, module_list):
        for descriptor in module_list.descriptors:
            module_id = descriptor.module_id
            module_class = descriptor.module_class

            if not self.instantiate_module(module_id, module_class):
                Logger.log_error(f"Failed to instantiate Module \"{module_id}\" (class: \"{module_class}\").\n"
                      f"The Module class was not registered to the ModuleFactory.")
                return False
        return True

    def initialize_modules(self, module_list, subscriber_publisher):
        for descriptor in module_list.descriptors:
            module_id = descriptor.module_id
            module_class = descriptor.module_class

            key = module_id
            if key not in self.__running_modules:
                Logger.log_error(f"Failed to initialize Module \"{module_id}\" (class: \"{module_class}\").\n"
                      f"The Module was not loaded.")
                return False

            module = self.__running_modules[key]

            Logger.log_info(f"Calling module.start() for Module \"{module.get_id()}\".")
            module.start(subscriber_publisher, descriptor.properties, self.main_thread_runnables_queue)
            Logger.log_info(f"Module \"{module.get_id()}\" has started.")

        return True

    def get_template_packages_of_modules(self):
        module_channels = {}
        for module_id in self.__running_modules.keys():
            template_packages_for_module = self.__channel_subscriber_publisher.get_channel_template_packages_for_module(module_id)
            module_channels[module_id] = template_packages_for_module
        return module_channels

    def start(self):
        Logger.log_info(str(self.__module_factory.get_registered_module_classes()))

        registered_module_classes = self.__module_factory.get_registered_module_classes()
        module_annotations = dict()

        for registered_module_class in registered_module_classes:

            module_annotator = ModuleAnnotator(registered_module_class)
            has_annotate_module_function = self.__module_factory.get_module_annotation(registered_module_class, module_annotator)
       

            if(has_annotate_module_function):
                module_annotations[registered_module_class] = module_annotator.get_annotations()

        module_list =  self.__module_dispatcher.get_module_list(registered_module_classes, module_annotations)
        Logger.log_info("Setting log severity level " + str(module_list.log_severity_level_for_host))
        Logger.log_severity_level_to_print = module_list.log_severity_level_for_host
        Logger.log_info(f"Received ModuleListResponse: {module_list}")
        if not self.instantiate_modules(module_list):
            Logger.log_fatal("ModuleDispatcher: Failed to instantiate Modules.")
            return False


        if not self.initialize_modules(module_list, self.__channel_subscriber_publisher):
            Logger.log_fatal("Failed to initialize Modules.")
            return False

        example_packages_of_modules = self.get_template_packages_of_modules()
        if not  self.__module_dispatcher.init_runtime(example_packages_of_modules):
            Logger.log_fatal("Failed to initialize runtime.")
            return False

        self.__from_module_dispatcher_queue_closed = False
        if not self.__module_dispatcher.send_receive_packages():
            Logger.log_fatal("Failed to set up input and output streams with middleware.")
            return False

        self.__from_module_dispatcher_queue = self.__module_dispatcher.get_from_dispatcher_queue()

        self.running = True
        self.read_from_module_dispatcher_thread = Thread(target=self.read_from_module_dispatcher)
        self.read_from_module_dispatcher_thread.start()

        return True
    
    def shutdown_modules(self):
        for module_name, module in self.__running_modules.items():
            Logger.log_info("Shutting down " + module_name)
            module.shutdown()
            Logger.log_info("Module has shutdown")
        
        self.__running_modules.clear()

    def stop(self):
         
        self.running = False
        self.__from_module_dispatcher_queue.cancel()
        self.read_from_module_dispatcher_thread.join()
        
        self.__channel_subscriber_publisher.reset()
    
    # def start_test(self):
    #     self.start()
    #     # io.ensure_future(self.start())

       
    #     # try:
    #     #     # Run the event loop
    #     #     io.get_event_loop().run_forever()
    #     # except KeyboardInterrupt:
    #     #     pass

    def split_host_module(self, addr):
        host_and_module = addr.split(":")

        if len(host_and_module) != 2:
            return None

        return host_and_module

    def on_data_package_received_from_module_dispatcher(self, data_package):
        if data_package.HasField('control_val') and data_package.control_val.ctrl_type != CtrlType.CTRL_UNSPECIFIED:
            self.handle_package_with_control_val(data_package)
            return

        channel_name = data_package.channel
        module_id = data_package.target_module

        Logger.log_info(f"ModuleManager received package with target for Module \"{module_id}\" on Channel \"{channel_name}\"")

        if self.__channel_subscriber_publisher is None:
            Logger.log_error("ModuleManager received DataPackage, however SubscriberPublisher is Null.")
            return

        if not self.__channel_subscriber_publisher.is_data_package_compatible_with_channel(data_package, module_id):
            payload_case = data_package.payload.message_type
            Logger.log_info(f"ModuleManager received package with target for Module \"{module_id}\" on Channel \"{channel_name}\", "
                            f"however the data type of payload of the package did not match the data type of the Channel.\n"
                            f"Expected payload type \"{self.__channel_subscriber_publisher.get_payload_case_of_channel(channel_name, module_id)}\" but got \"{payload_case}")
            return

        subscriber_list = self.__channel_subscriber_publisher.get_subscriber_instances_of_module(channel_name, module_id)

        if subscriber_list is None:
            Logger.log_info(f"ModuleManager received package with target for Module \"{module_id}\" on Channel \"{channel_name}\",\n"
                            f"however a Subscriber of the Module for this Channel was not found. The Module has no Subscriber for this Channel.")
        for subscriber in subscriber_list:
            subscriber.on_new_data(data_package)

    def handle_package_with_control_val(self, packet):
        if packet.control_val.ctrl_type == CtrlType.CTRL_CONNECTED_TO_REMOTE_SERVER:
            for module_id, module in self.__running_modules.items():
                module.notify_connected_to_remote_server()

            self.__on_connected_to_remote_server()
        elif packet.control_val.ctrl_type == CtrlType.CTRL_DISCONNECTED_FROM_REMOTE_SERVER:
            for module_id, module in self.__running_modules.items():
                module.notify_disconnected_from_remote_server()
            
            self.__on_disconnected_from_remote_server()
        elif packet.control_val.ctrl_type == CtrlType.CTRL_UNLOAD_MODULES:
            Logger.log_info("Python ModuleManager received CTRL_UNLOAD_MODULES")
            self.shutdown_modules()


            response = DataPackage()
            ctrl_package = response.control_val

            ctrl_package.ctrl_type = CtrlType.CTRL_UNLOAD_MODULES_DONE
            ctrl_package.runtime = Runtime.RUNTIME_PYTHON
            response.source_host = packet.target_host
            response.target_host = packet.source_host

            self.__to_module_dispatcher_queue.put(response)

            Logger.log_info("Unloading Modules done")
        elif packet.control_val.ctrl_type == CtrlType.CTRL_RESTART_RUNTIME:
            
            Logger.log_info("Python ModuleManager received CTRL_RESTART_RUNTIME")
            self.restart_thread = Thread(target=self.restart)
            self.restart_thread.start()

            self.__restart_control_package = packet
        elif packet.control_val.ctrl_type == CtrlType.CTRL_REQUEST_MODULE_ANNOTATIONS_RESPONSE:

            host_name = packet.source_host # Sender of the module annotations
            self.__module_annotations_for_host[host_name] = packet.control_val.module_annotations
            self.__module__anotations_updated_for_host[host_name] = True

        elif packet.control_val.ctrl_type == CtrlType.CTRL_ON_NEW_CONFIG_PAYLOAD_DATA:
            payload = packet.control_val.config_upload_payload
            
            self.inject_modules_from_config_upload_payload(payload)

        elif packet.control_val.ctrl_type == CtrlType.CTRL_LOG_SINK_LOG_MESSAGE_STREAM:
            if self.__log_sink_log_message_callback != None:
                self.__log_sink_log_message_callback(packet)

        else:
            Logger.log_warning(f"ModuleManager received package with unsupported control val {packet.control_val.ctrl_type}")

    def read_from_module_dispatcher(self):
        while self.running:
            Logger.log_info("on read from module dispatcher")
            try:
                for data_package in self.__from_module_dispatcher_queue:
                    if data_package is not None:
                        self.on_data_package_received_from_module_dispatcher(data_package)
                    else:
                        pass
            except Exception as e:
                Logger.log_error("RUNTIME_PYTHON got error during read from module_dispatcher. Terminating:" + str(e) + str(e.__traceback__))
                traceback.print_exc() 
                break
        
        self.__from_module_dispatcher_queue_closed = True


    

    def restart(self):
        Logger.log_info("Stopping ModuleManager")
        self.stop()
        Logger.log_info("Stopping ModuleDispatcher")
        self.__module_dispatcher.shutdown()

        Logger.log_info("Waiting42")
        while not self.__from_module_dispatcher_queue_closed:
            Logger.log_info("Waiting")
        time.sleep(2)
        Logger.log_info("Restarting ModuleManager and ModuleDispatcher")
        self.start()

        response = DataPackage()
        ctrl_package = response.control_val

        ctrl_package.ctrl_type = CtrlType.CTRL_RESTART_RUNTIME_DONE
        ctrl_package.runtime = Runtime.RUNTIME_PYTHON
        response.source_host = self.__restart_control_package.target_host
        response.target_host = self.__restart_control_package.source_host

        self.__to_module_dispatcher_queue.put(response)

    def update_module_annotations_of_host(self, host):
        self.__module__anotations_updated_for_host[host] = False
        
        package = DataPackage()
        package.target_host = host
        # package.source_host = ... will be set by middleware
        ctrl_package = package.control_val

        ctrl_package.ctrl_type = CtrlType.CTRL_REQUEST_MODULE_ANNOTATIONS
        ctrl_package.runtime = Runtime.RUNTIME_PYTHON
   

        self.__to_module_dispatcher_queue.put(package)

    def are_module_annotations_of_host_updated(self, host):
        return self.__module__anotations_updated_for_host[host]
    
    def get_module_annotations_of_host(self, host):
        return self.__module_annotations_for_host[host]
    
    # dict[str, List]
    # dict(code, module_names)
    def inject_new_modules(self, module_descrption: dict):

        

        for code_name in module_descrption:
            code, module_names = module_descrption[code_name]
            if not self.__module_factory.inject_claid_modules_from_python_file(code_name, code, module_names):
                return False
            
       
        self.__module_factory.Logger.log_info_registered_modules()

        return True


    def inject_modules_from_config_upload_payload(self, payload):

        Logger.log_info("Injecting modules from payload: " + str(payload.modules_to_inject))
        module_injections = dict()

        for module_injection in payload.modules_to_inject:
            if module_injection.runtime != Runtime.RUNTIME_PYTHON:
                continue

            Logger.log_info("Checking for module injections")
            if module_injection.module_file not in module_injections:
                module_injections[module_injection.module_file] = list()

            module_injections[module_injection.module_file].append(module_injection.module_name)

        Logger.log_info("Module injections: " + str(module_injections))
        for entry in module_injections:
            if not self.__module_factory.inject_claid_modules_from_python_file(payload.payload_data_path, entry, module_injections[entry]):
                Logger.log_error("Failed to inject modules {}".format(module_injections[entry]))
                return False
            
        return True

    def register_on_connected_to_server_callback(self, callback):
        self.__on_connected_to_remote_server_callbacks.append(callback)

    def register_on_disconnected_from_server_callback(self, callback):
        self.__on_disconnected_from_remote_server_callbacks.append(callback)

    def __on_connected_to_remote_server(self):
        for callback in self.__on_connected_to_remote_server_callbacks:
            callback()

    def __on_disconnected_from_remote_server(self):
        for callback in self.__on_disconnected_from_remote_server_callbacks:
            callback()

    def upload_config_to_host(self, host_name: str, config: CLAIDConfig):
        
        self.upload_config_to_host_with_payload(host_name, config, ConfigUploadPayload())


    def upload_config_to_host_with_payload(self, host_name: str, config: CLAIDConfig, config_payload: ConfigUploadPayload):
        package = DataPackage()
        package.target_host = host_name
        # package.source_host = ... will be set by middleware
        ctrl_package = package.control_val

        ctrl_package.ctrl_type = CtrlType.CTRL_UPLOAD_CONFIG_AND_DATA
        ctrl_package.runtime = Runtime.RUNTIME_PYTHON

        config_payload.config.CopyFrom(config)
        ctrl_package.config_upload_payload.CopyFrom(config_payload)
   
        self.__to_module_dispatcher_queue.put(package)

    def post_log_message(self, log_message: LogMessage):

        if not self.running:
            return
        package = DataPackage()
        # package.source_host = ... will be set by middleware

        ctrl_package = package.control_val

        ctrl_package.ctrl_type = CtrlType.CTRL_LOCAL_LOG_MESSAGE
        ctrl_package.runtime = Runtime.RUNTIME_PYTHON

        
        ctrl_package.log_message.CopyFrom(log_message)
        self.__to_module_dispatcher_queue.put(package)

    def subscribe_log_sink_log_messages(self, callback):
        self.__log_sink_log_message_callback = callback

        ctrl_package = package.control_val

        ctrl_package.ctrl_type = CtrlType.CTRL_SUBSCRIBE_TO_LOG_SINK_LOG_MESSAGE_STREAM
        ctrl_package.runtime = Runtime.RUNTIME_PYTHON

        self.__to_module_dispatcher_queue.put(package)

    def unsubscribe_log_sink_log_messages(self):
        self.__log_sink_log_message_callback = None

        ctrl_package = package.control_val

        ctrl_package.ctrl_type = CtrlType.CTRL_UNSUBSCRIBE_FROM_LOG_SINK_LOG_MESSAGE_STREAM
        ctrl_package.runtime = Runtime.RUNTIME_PYTHON

        self.__to_module_dispatcher_queue.put(package)