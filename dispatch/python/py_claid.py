import ctypes
import sys
from CLAID import CLAID
from module.module_factory import ModuleFactory
from module.module import Module
# sys.path.append(pathlib.Path().absolute() / "external/google/protobuf/internal")

from module.type_mapping.type_mapping import TypeMapping
from module.type_mapping.mutator import Mutator

from dispatch.proto.claidservice_pb2 import DataPackage
from typing import List
import numpy as np

package = DataPackage()


from local_dispatching.module_dispatcher import ModuleDispatcher

value = {str:int}

dispatcher = ModuleDispatcher("unix:///tmp/test.grpc")


# mutator = TypeMapping.get_mutator(value)

# value = {"test": 42, "test2" : 33}
# mutator.set_package_payload(package, value)
# print(package)
# print(mutator.get_package_payload(package))


# import pathlib

# module_factory = ModuleFactory()
# claid = CLAID()
# claid.start("/Users/planger/Development/ModuleAPIV2/dispatch/test/remote_dispatching_test.json", "test_client", "user", "device", module_factory)
# # socket = "unix:///tmp/claid_socket.grpc".encode('utf-8')
# # config = "/Users/planger/Development/ModuleAPIV2/dispatch/test/remote_dispatching_test.json".encode('utf-8')
# # client = "test_client".encode('utf-8')

# # # const char* socket_path, const char* config_file, const char* host_id, const char* user_id, const char* device_id
# # lib.start_core(socket, config, client, "test", "test", "test")