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

import platform
import os

import time

class CLAID():

    
    claid_c_lib = int(0)
    claid_c_lib_loaded = False

    @classmethod
    def __load_claid_library(clz):
        if CLAID.claid_c_lib_loaded:
            return

        print("Loading CLAID lib")
        print(type(CLAID.claid_c_lib))
        if(isinstance(CLAID.claid_c_lib, int)):

            # Get the current operating system
            current_os = platform.system()
            platform_library_extension = ""
            # Check the operating system
            if current_os == "Linux":
                platform_library_extension = ".so"
            elif current_os == "Darwin":
                platform_library_extension = ".dylib"
            elif current_os == "Windows":
                platform_library_extension = ".dll"


            if platform_library_extension == "":
                raise Exception("Failed to load CLAID library into Python. Unsupported OS \"{}\". Supported are only Linux, Darwin (macOS) and Windows").format(current_os)

            libname = pathlib.Path().absolute() / "dispatch/core/libclaid_capi{}".format(platform_library_extension)
            if not os.path.isfile(libname):
                current_file_path = str(os.path.dirname(os.path.abspath(__file__)))
                libname = os.path.join(current_file_path, "dispatch/core/libclaid_capi{}".format(platform_library_extension))
            print("Calling loadlibrary")
            CLAID.claid_c_lib = ctypes.CDLL(libname)
            print("Loading CLAID load library")

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
        CLAID.claid_c_lib_loaded = True
    
        print("CLAID c lib loaded")
    def __init__(self, module_injection_storage_path = None):
        self.__handle = 0
        self.__cpp_runtime_handle = 0
        self.__started = False
        self.__main_thread_queue = ThreadSafeChannel()



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
        
        if not self.attach_python_runtime(socket_path, module_factory):
            raise Exception("Failed to attach Python runtime")

        Logger.log_info("Successfully started CLAID")

        # This will block indefinitely and wakes up when there are runnables to execute
        self.process_runnables_blocking()

    def start(self, config_file_path, host_id, user_id, device_id, module_factory):
        return self.startCustomSocket("unix:///tmp/claid_socket.grpc", config_file_path, host_id, user_id, device_id, module_factory)

    def attach_python_runtime(self, socket_path, module_factory):

        self.__module_dispatcher = ModuleDispatcher(socket_path)

        self.__module_manager = ModuleManager(self.__module_dispatcher, module_factory, self.__main_thread_queue)
        print("starting Python runtime")

        return self.__module_manager.start()
    
    def load_new_config_test(self, config_path):
        print("Load new config 1 ", config_path)
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

    def get_output_channels_of_module(self, module_annotation):
        channels = list()
        for channel in module_annotation.channel_definition:
            if channel.source_module != "":
                channels.append(channel.channel)

        return channels
    
    def process_runnables_blocking(self):

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


    # Allows an external entity to subscribe to log messages received by the CLAID framework.
    def subscribe_for_log_messages(self, callback):
        pass

    def register_on_connected_to_server_callback(self, callback):
        self.__module_manager.register_on_connected_to_server_callback(callback)

    def register_on_disconnected_from_server_callback(self, callback):
        self.__module_manager.register_on_disconnected_from_server_callback(callback)

    def upload_config_to_host(self, host_name: str, config: CLAIDConfig):
        self.__module_manager.upload_config_to_host(self, host_name, config)

    def enable_designer_mode(self):
        CLAID.claid_c_lib.enable_designer_mode(self.__handle)

    def disable_designer_mode(self):
        CLAID.claid_c_lib.disable_designer_mode(self.__handle)

    def set_payload_data_path(self, path: str):
        CLAID.claid_c_lib.set_payload_data_path(self.__handle, string_to_c_string(path))

    def get_payload_data_path(self) -> str:
        return CLAID.claid_c_lib.get_payload_data_path(self.__handle).decode('utf-8')