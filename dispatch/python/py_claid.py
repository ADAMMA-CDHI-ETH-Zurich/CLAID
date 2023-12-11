import ctypes
import sys
from CLAID import CLAID
from module.module_factory import ModuleFactory
from module.module import Module
# sys.path.append(pathlib.Path().absolute() / "external/google/protobuf/internal")


import pathlib


print(pathlib.Path().absolute())
libname = pathlib.Path().absolute() / "dispatch/core/libclaid_capi.dylib"
lib = ctypes.CDLL(libname)

print("Starting core")

module_factory = ModuleFactory()
module_factory.register_module(Test)
claid = CLAID()
claid.start("/Users/planger/Development/ModuleAPIV2/dispatch/test/remote_dispatching_test.json", "test_client", "user", "device", module_factory)
# socket = "unix:///tmp/claid_socket.grpc".encode('utf-8')
# config = "/Users/planger/Development/ModuleAPIV2/dispatch/test/remote_dispatching_test.json".encode('utf-8')
# client = "test_client".encode('utf-8')

# # const char* socket_path, const char* config_file, const char* host_id, const char* user_id, const char* device_id
# lib.start_core(socket, config, client, "test", "test", "test")