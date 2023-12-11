import dispatch.proto.claidconfig_pb2
import dispatch.proto.sensor_data_types_pb2
from helpers.c_binding_helpers import strings_to_c_strings
import ctypes
import pathlib
from module.module_factory import ModuleFactory


class CLAID():

    
    claid_c_lib = int(0)
    
    def __init__(self):
        self.__handle = 0
        self.__cpp_runtime_handle = 0
        self.__started = False

        print(type(CLAID.claid_c_lib))
        if(isinstance(CLAID.claid_c_lib, int)):
            libname = pathlib.Path().absolute() / "dispatch/core/libclaid_capi.dylib"
            CLAID.claid_c_lib = ctypes.CDLL(libname)


    def startCustomSocket(self, socket_path, config_file_path, host_id, user_id, device_id, module_factory):
        
        if(self.__started):
            raise Exception("Failed to start CLAID, start was called twice")

        self.__started = True


        socket_path, config_file_path, host_id, user_id, device_id = strings_to_c_strings([socket_path, config_file_path, host_id, user_id, device_id])

        self.__handle = CLAID.claid_c_lib.start_core(socket_path, config_file_path, host_id, user_id, device_id)

    def start(self, config_file_path, host_id, user_id, device_id, module_factory):
        self.startCustomSocket("unix:///tmp/claid_socket.grpc", config_file_path, host_id, user_id, device_id, module_factory)
