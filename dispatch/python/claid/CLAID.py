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

import dispatch.proto.sensor_data_types_pb2
from helpers.c_binding_helpers import strings_to_c_strings, string_to_c_string
import ctypes
import pathlib
from module.module_factory import ModuleFactory
from logger.logger import Logger

from local_dispatching.module_dispatcher import ModuleDispatcher
from local_dispatching.module_manager import ModuleManager
from module.module_factory import ModuleFactory
from module.thread_safe_channel import ThreadSafeChannel

from module.scheduling.function_runnable import FunctionRunnable

from module.scheduling.schedule_once import ScheduleOnce
from module.scheduling.schedule_immediately_indefinitely import ScheduleImmediatelyIndefinitely
from module.scheduling.external_scheduled_runnable import ExternalScheduledRunnable

from dispatch.proto.claidservice_pb2 import LogMessage, LogMessageSeverityLevel

import platform
import os
import time
import threading

class CLAID():

    
    claid_c_lib = int(0)
    claid_c_lib_loaded = False

    @classmethod
    def __load_claid_library(clz):
        if CLAID.claid_c_lib_loaded:
            return

        if(isinstance(CLAID.claid_c_lib, int)):

            # Get the current operating system

            claid_clib_path = CLAID.get_claid_clib_storage_path()
            print("CLAID PATH ", claid_clib_path)

            current_file_path = os.path.abspath(__file__)

            # Get the directory containing the currently executed Python file
            current_directory = os.path.dirname(current_file_path)

            if not os.path.isfile(claid_clib_path):
                alternative_default_path = "{}/dispatch/core/{}".format(os.getcwd(), CLAID.get_claid_clib_name())
                print("alternative path: {}".format(alternative_default_path))

                if os.path.isfile(alternative_default_path):
                    claid_clib_path = alternative_default_path
                else:
                    raise ValueError("Failed to load CLAID library from {}".format(claid_clib_path))
                
            CLAID.claid_c_lib = ctypes.CDLL(claid_clib_path)

            # Required, otherwise claid_c_lib.attach_cpp_runtime will fail (same for shutdown_core etc).
            CLAID.claid_c_lib.start_core.restype = ctypes.c_void_p
            CLAID.claid_c_lib.start_core.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
            CLAID.claid_c_lib.attach_cpp_runtime.argtypes = [ctypes.c_void_p]
            CLAID.claid_c_lib.shutdown_core.argtypes = [ctypes.c_void_p]

            CLAID.claid_c_lib.get_socket_path.argtypes = [ctypes.c_void_p]
            CLAID.claid_c_lib.get_socket_path.restype = ctypes.c_char_p

            CLAID.claid_c_lib.get_host_id.argtypes = [ctypes.c_void_p]
            CLAID.claid_c_lib.get_host_id.restype = ctypes.c_char_p

            CLAID.claid_c_lib.get_user_id.argtypes = [ctypes.c_void_p]
            CLAID.claid_c_lib.get_user_id.restype = ctypes.c_char_p

            CLAID.claid_c_lib.get_device_id.argtypes = [ctypes.c_void_p]
            CLAID.claid_c_lib.get_device_id.restype = ctypes.c_char_p

            CLAID.claid_c_lib.load_new_config.restype = ctypes.c_bool
            CLAID.claid_c_lib.load_new_config.argtypes = [ctypes.c_void_p, ctypes.c_char_p]


            CLAID.claid_c_lib.enable_designer_mode.argtypes = [ctypes.c_void_p]
            CLAID.claid_c_lib.enable_designer_mode.restype = None

            CLAID.claid_c_lib.disable_designer_mode.argtypes = [ctypes.c_void_p]
            CLAID.claid_c_lib.disable_designer_mode.restype = None

            CLAID.claid_c_lib.set_payload_data_path.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
            CLAID.claid_c_lib.set_payload_data_path.restype = None

            CLAID.claid_c_lib.get_payload_data_path.argtypes = [ctypes.c_void_p]
            CLAID.claid_c_lib.get_payload_data_path.restype = ctypes.c_char_p

            CLAID.claid_c_lib.get_log_sink_severity_level.argtypes = [ctypes.c_void_p]
            CLAID.claid_c_lib.get_log_sink_severity_level.restype = ctypes.c_int
        CLAID.claid_c_lib_loaded = True
    
        print("CLAID c lib loaded")

    @staticmethod 
    def get_claid_clib_name():
        current_os = platform.system()
       

        
        platform_library_extension = ""
        # Check the operating system
        if current_os == "Linux":
            platform_library_extension = ".so"


        elif current_os == "Darwin":
            platform_library_extension = ".dylib"

            
        elif current_os == "Windows":
            platform_library_extension = ".dll"

        return "libclaid_capi{}".format(platform_library_extension)

    @staticmethod
    def get_claid_clib_storage_path():
        current_file_path = os.path.abspath(__file__)

        # Get the directory containing the currently executed Python file
        current_directory = os.path.dirname(current_file_path)

        current_os = platform.system()
        architecture = platform.machine()
        
        if architecture == "aarch64":
            architecture = "arm64"

        libname = os.path.join(current_directory, "dispatch/core/{}/{}/{}".format(current_os, architecture, CLAID.get_claid_clib_name()))
        return libname

    def __init__(self, module_injection_storage_path = None):
        self.__handle = 0
        self.__cpp_runtime_handle = 0
        self.__started = False
        self.__main_thread_queue = ThreadSafeChannel()
        self.__module_manager_thread = None



    def startCustomSocket(self, socket_path, config_file_path, host_id, user_id, device_id, module_factory):
        CLAID.__load_claid_library()

        Logger.log_info("Starting CLAID")

        if(self.__started):
            raise Exception("Failed to start CLAID, start was called twice")

        self.__started = True

        socket_path, config_file_path, host_id, user_id, device_id = strings_to_c_strings([socket_path, config_file_path, host_id, user_id, device_id])

        self.__handle = CLAID.claid_c_lib.start_core(socket_path, config_file_path, host_id, user_id, device_id)

        if(self.__handle == 0):
            raise Exception("Failed to start CLAID middleware.")
        
        Logger.log_info("Successfully started CLAID middleware")
        Logger.log_info("Attaching C++ runtime")

        self.__cpp_runtime_handle = CLAID.claid_c_lib.attach_cpp_runtime(self.__handle)

        if(self.__cpp_runtime_handle == 0):
            raise Exception("Failed to start CLAID, could not start C++ runtime")
        
        Logger.claid_instance = self
        Logger.log_info("Successfully started CLAID")
        self.attach_python_runtime(socket_path, module_factory)
           


        # This will block indefinitely and wakes up when there are runnables to execute

    def start(self, config_file_path, host_id, user_id, device_id, module_factory):
        return self.startCustomSocket("unix:///tmp/claid_socket.grpc", config_file_path, host_id, user_id, device_id, module_factory)

    def attach_python_runtime(self, socket_path, module_factory):

        self.__module_dispatcher = ModuleDispatcher(socket_path)

        self.__module_manager = ModuleManager(self.__module_dispatcher, module_factory, self.__main_thread_queue)
        print("starting Python runtime")

        # In Python, we have to launch the ModuleManager in a separate thread, because each Module will
        # forward all of it's runnables to the main thread. If the ModuleManager would also be called from the main thread,
        # then it would block during module.start(), as the Modules inject a runnable with their initialize function to the main thread queue -> deadlock.
        self.__module_manager_thread = threading.Thread(target=self.__module_manager.start)
        self.__module_manager_thread.start()
        Logger.log_info("Started ModuleManager thread")
        self.__module_factory = module_factory
        self.__started = True
        self.process_runnables_blocking()

    def load_new_config_test(self, config_path):
        Logger.log_info("Load new config 1 ", config_path)
        config_path_c = string_to_c_string(config_path)
        print("Load new config 3", self.__handle, config_path)
        res = CLAID.claid_c_lib.load_new_config(self.__handle, config_path_c)
        print("Load new config 4")
        return True
    

    def update_module_annotations_of_host(self, host_name):
        self.__module_manager.update_module_annotations_of_host(host_name)

    def are_module_annotations_of_host_updated(self, host_name):
        return self.__module_manager.are_module_annotations_of_host_updated(host_name)
    
    def get_module_annotations_of_host(self, host_name):
        return self.__module_manager.get_module_annotations_of_host(host_name)

    def inject_new_modules(self, module_descriptions: dict):
        return self.__module_manager.inject_new_modules(module_descriptions)
    
    def get_current_host_id(self):
        return CLAID.claid_c_lib.get_host_id(self.__handle).decode('utf-8')

    def get_available_modules_on_this_host(self):
        return self.get_available_modules_for_host(self.get_current_host_id())

    def get_available_modules_for_host(self, host_name):

        if not self.__started:
            raise RuntimeError("Cannot get available modules. CLAID is not running, you have to start it first.")

        self.update_module_annotations_of_host(host_name)
        start_time = int(round(time.time() * 1000))

        while not self.are_module_annotations_of_host_updated(host_name):
            
            # Make sure we are not blocking forever, if the target host is not reachable.
            current_time = int(round(time.time() * 1000))
            if(current_time - start_time > 3000):
                return None
            


        return self.get_module_annotations_of_host(host_name)
    
    def hello_world(self):
        CLAID.__load_claid_library()
        CLAID.claid_c_lib.hello_world()


    def get_input_channels_of_module(self, module_annotation):
        channels = list()
        for channel in module_annotation.channel_definition:
            if channel.target_module != "":
                channels.append(channel.channel)

        return channels
    
    def get_input_channel_types_of_module(self, module_annotation):
        channel_types = dict() # str: DataPackage (where DataPackage indicates the Channel data type)

        for channel in module_annotation.channel_definition:
            if channel.target_module != "":
                channel_types[channel.channel] = channel

        return channel_types


    def get_output_channels_of_module(self, module_annotation):
        channels = list()
        for channel in module_annotation.channel_definition:
            if channel.source_module != "":
                channels.append(channel.channel)

        return channels
    
    def get_output_channel_types_of_module(self, module_annotation):
        channel_types = dict() # str: DataPackage (where DataPackage indicates the Channel data type)

        for channel in module_annotation.channel_definition:
            if channel.source_module != "":
                channel_types[channel.channel] = channel

        return channel_types
    
    def process_runnables_blocking(self):

        print("Processing runnables ", self.__started)
        while self.__started:

            scheduled_runnable = self.__main_thread_queue.get()

            if scheduled_runnable is None:
                continue

            if scheduled_runnable.runnable.catch_exceptions:
                try:
                    Logger.log_info(
                    f"Running runnable! Remaining runnables: {len(self.scheduled_runnables)}"
                    )
                    scheduled_runnable.runnable.run()
                except Exception as e:
                    scheduled_runnable.runnable.set_exception(str(e))
            else:
                scheduled_runnable.runnable.run()

            scheduled_runnable.runnable.was_executed = True

            if isinstance(scheduled_runnable, ExternalScheduledRunnable):
                # Reschedule the runnable is only done here for ExternalScheduledRunnables (i.e., 
                # Runnables outside of the CLAID framework). All other runnables are scheduled inside
                # each Modules individual RunnableDispatcher.

                if scheduled_runnable.schedule.does_runnable_have_to_be_repeated():
                    self.__main_thread_queue.put(scheduled_runnable)

    # Allows to inject internal functions as runnables in the __main_thread_queue.
    # This can be required for certain frameworks to run on the same (main) thread as CLAID,
    # e.g. for GUI frameworks like Qt.
    def register_external_function_on_claid_thread_run_once(self, function):
        function_runnable = FunctionRunnable(function)

        
        runnable =  ExternalScheduledRunnable(
                function_runnable,
                schedule=ScheduleOnce.now()
            )

        self.__main_thread_queue.put(runnable)

    def register_external_function_on_claid_thread_repeat_indefinitely(self, function):
        function_runnable = FunctionRunnable(function)

        
        runnable = ExternalScheduledRunnable(
                function_runnable,
                schedule=ScheduleImmediatelyIndefinitely.startNow()
            )
        
        self.__main_thread_queue.put(runnable)


    

    def register_on_connected_to_server_callback(self, callback):
        self.__module_manager.register_on_connected_to_server_callback(callback)

    def register_on_disconnected_from_server_callback(self, callback):
        self.__module_manager.register_on_disconnected_from_server_callback(callback)

    def upload_config_to_host_with_payload(self, host_name: str, config, config_payload):
        self.__module_manager.upload_config_to_host_with_payload(host_name, config, config_payload)


    def upload_config_to_host(self, host_name: str, config):
        self.__module_manager.upload_config_to_host(host_name, config)

    def enable_designer_mode(self):
        CLAID.claid_c_lib.enable_designer_mode(self.__handle)

    def disable_designer_mode(self):
        CLAID.claid_c_lib.disable_designer_mode(self.__handle)

    def set_payload_data_path(self, path: str):
        CLAID.claid_c_lib.set_payload_data_path(self.__handle, string_to_c_string(path))

    def get_payload_data_path(self) -> str:
        return CLAID.claid_c_lib.get_payload_data_path(self.__handle).decode('utf-8')
    
    def get_log_sink_severity_level(self) -> LogMessageSeverityLevel:
        value = CLAID.claid_c_lib.get_log_sink_severity_level(self.__handle)
        return LogMessageSeverityLevel.Value(LogMessageSeverityLevel.keys()[value])
    
    def post_log_message(self, log_message: LogMessage):
        self.__module_manager.post_log_message(log_message)

    # Allows an external entity to subscribe to log messages received by the CLAID framework.
    def subscribe_log_sink_log_messages(self, callback):
        self.__module_manager.subscribe_for_log_sink_log_messages(callback)

    def unsubscribe_log_sink_log_messages(self):
        self.__module_manager.unsubscribe_log_sink_log_messages()

    def get_module_factory(self):
        return self.__module_factory