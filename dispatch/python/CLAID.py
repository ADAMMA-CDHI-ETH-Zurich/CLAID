import dispatch.proto.claidconfig_pb2
import dispatch.proto.sensor_data_types_pb2
from helpers.c_binding_helpers import strings_to_c_strings
import ctypes
import pathlib
from module.module_factory import ModuleFactory
from logger.logger import Logger

from local_dispatching.module_dispatcher import ModuleDispatcher
from module.module_factory import ModuleFactory

import platform

class CLAID():

    
    claid_c_lib = int(0)
    
    def __init__(self):
        self.__handle = 0
        self.__cpp_runtime_handle = 0
        self.__started = False

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
            CLAID.claid_c_lib = ctypes.cdll.LoadLibrary(libname)

            # Required, otherwise claid_c_lib.attach_cpp_runtime will fail (same for shutdown_core etc).
            # CLAID.claid_c_lib.start_core.restype = ctypes.c_void_p
            # CLAID.claid_c_lib.start_core.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
            # CLAID.claid_c_lib.attach_cpp_runtime.argtypes = [ctypes.c_void_p]
            # CLAID.claid_c_lib.shutdown_core.argtypes = [ctypes.c_void_p]

            # CLAID.claid_c_lib.get_socket_path.argtypes = [ctypes.c_void_p]
            # CLAID.claid_c_lib.get_socket_path.restype = ctypes.c_char_p


    def startCustomSocket(self, socket_path, config_file_path, host_id, user_id, device_id, module_factory):
        
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
        
        if not self.attach_python_runtime(socket_path):
            raise Exception("Failed to attach Python runtime")

        Logger.log_info("Successfully started CLAID")


    def start(self, config_file_path, host_id, user_id, device_id, module_factory):
        self.startCustomSocket("unix:///tmp/claid_socket.grpc", config_file_path, host_id, user_id, device_id, module_factory)

    def attach_python_runtime(self, socket_path):

        if(self.__started):
            Logger.log_error("CLAID middleware start was called twice!")
            return False

        self.__module_dispatcher = ModuleDispatcher(socket_path)

        self.__module_manager = ModuleManager(moduleDispatcher, factory)
