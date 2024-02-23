import dispatch.proto.sensor_data_types_pb2
from helpers.c_binding_helpers import strings_to_c_strings, string_to_c_string
import ctypes
import pathlib
from module.module_factory import ModuleFactory
from logger.logger import Logger

from local_dispatching.module_dispatcher import ModuleDispatcher
from local_dispatching.module_manager import ModuleManager
from module.module_factory import ModuleFactory

import platform
import os
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
            CLAID.claid_c_lib = ctypes.cdll.LoadLibrary(libname)

            # Required, otherwise claid_c_lib.attach_cpp_runtime will fail (same for shutdown_core etc).
            CLAID.claid_c_lib.start_core.restype = ctypes.c_void_p
            CLAID.claid_c_lib.start_core.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
            CLAID.claid_c_lib.attach_cpp_runtime.argtypes = [ctypes.c_void_p]
            CLAID.claid_c_lib.shutdown_core.argtypes = [ctypes.c_void_p]

            CLAID.claid_c_lib.get_socket_path.argtypes = [ctypes.c_void_p]
            CLAID.claid_c_lib.get_socket_path.restype = ctypes.c_char_p

            CLAID.claid_c_lib.load_new_config.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
            CLAID.claid_c_lib.load_new_config.restype = ctypes.c_bool

            CLAID.claid_c_lib.enable_designer_mode.argtypes = [ctypes.c_void_p]
            CLAID.claid_c_lib.enable_designer_mode.restype = None

            CLAID.claid_c_lib.disable_designer_mode.argtypes = [ctypes.c_void_p]
            CLAID.claid_c_lib.disable_designer_mode.restype = None

            
        CLAID.claid_c_lib_loaded = True
    
    def __init__(self, module_injection_storage_path = None):
        self.__handle = 0
        self.__cpp_runtime_handle = 0
        self.__started = False
        self.module_injection_storage_path = module_injection_storage_path



    def startCustomSocket(self, socket_path, config_file_path, host_id, user_id, device_id, module_factory):
        CLAID.__load_claid_library()

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

        return True

    def start(self, config_file_path, host_id, user_id, device_id, module_factory):
        return self.startCustomSocket("unix:///tmp/claid_socket.grpc", config_file_path, host_id, user_id, device_id, module_factory)

    def attach_python_runtime(self, socket_path, module_factory):

        self.__module_dispatcher = ModuleDispatcher(socket_path)

        self.__module_manager = ModuleManager(self.__module_dispatcher, module_factory, self.module_injection_storage_path)
        print("starting Python runtime")

        return self.__module_manager.start()
    
    def load_new_config_test(self, config_path):
        print("Load new config 1 ", config_path)
        config_path_c = string_to_c_string(config_path)
        print("Load new config 3", self.__handle, config_path)
        res = CLAID.claid_c_lib.load_new_config(self.__handle, config_path_c)
        print("Load new config 4")
        return True
    

    def update_module_annotations(self):
        self.__module_manager.update_module_annotations()

    def are_module_annotations_updated(self):
        return self.__module_manager.are_module_annotations_updated()
    
    def get_module_annotations(self):
        return self.__module_manager.get_module_annotations()
    
    def inject_new_modules(self, module_descriptions: dict):
        return self.__module_manager.inject_new_modules(module_descriptions)
    
    def get_available_modules(self):

        if not self.__started:
            raise RuntimeError("Cannot get available modules. CLAID is not running, you have to start it first.")

        self.update_module_annotations()
        while not self.are_module_annotations_updated():
            pass

        return self.get_module_annotations()
    
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