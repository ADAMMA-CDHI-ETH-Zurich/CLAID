import ctypes
import sys
import dispatch.proto.claidconfig_pb2
import dispatch.proto.sensor_data_types_pb2
import ctypes
import pathlib
from module.module_factory import ModuleFactory
from logger.logger import Logger
import sys,os
sys.path.append(os.getcwd() + "/dispatch/python")
from local_dispatching.module_dispatcher import ModuleDispatcher
from module.module_factory import ModuleFactory

import platform
from CLAID import CLAID
from module.module import Module

# from module.module_factory import ModuleFactory
# from module.module import Module
# # sys.path.append(pathlib.Path().absolute() / "external/google/protobuf/internal")
# import pathlib
# from module.type_mapping.type_mapping import TypeMapping
# from module.type_mapping.mutator import Mutator



# value = {str:int}

# dispatcher = ModuleDispatcher("unix:///tmp/test.grpc")

from datetime import datetime, timedelta

class TestModule(Module):
    def __init__(self):
        super().__init__()
        pass

    @staticmethod
    def annotate_module(annotator):
        annotator.set_module_description("This Module can...")
        annotator.describe_property("samplingRate", "Allows to")
        annotator.describe_publish_channel("MyChannel", int(42), "")
        return {"test": "test"}

    def initialize(self, properties):
        Logger.log_info("TestModule Initialize")
        self.output_channel = self.publish("TestChannel", int(0))
        self.ctr = 0
        self.register_periodic_function("Test", self.periodic_function, timedelta(milliseconds=1000))

    def periodic_function(self):
        Logger.log_info("PeriodicFunctio ")
        self.output_channel.post(self.ctr)
        self.ctr+=1


class TestModule2(Module):
    def __init__(self):
        super().__init__()
        pass

    def initialize(self, properties):
        Logger.log_info("TestModule Initialize")
        self.input_channel = self.subscribe("TestChannel", int(0), self.on_data)
        self.ctr = 0

    def on_data(self, data):
        
        print("Data", data.get_data())

module_factory = ModuleFactory()
# module_factory.register_module(TestModule)
# module_factory.register_module(TestModule2)
claid = CLAID()
claid.start("/Users/planger/Development/ModuleAPIV2/dispatch/python/test_config.json", "test_client", "user", "device", module_factory)

claid.update_module_annotations()
while(not claid.are_module_annotations_updated()):
    pass

print(claid.get_module_annotations())
while(True):
    pass
# socket = "unix:///tmp/claid_socket.grpc".encode('utf-8')
# config = "/Users/planger/Development/ModuleAPIV2/dispatch/test/remote_dispatching_test.json".encode('utf-8')
# client = "test_client".encode('utf-8')
# libname = "/Users/planger/Development/ModuleAPIV2/bazel-bin/dispatch/core/libclaid_capi.dylib"
# lib = ctypes.CDLL(libname)
# # const char* socket_path, const char* config_file, const char* host_id, const char* user_id, const char* device_id
# lib.start_core(socket, config, client, "test", "test", "test")

# mutator = TypeMapping.get_mutator(value)

# value = {"test": 42, "test2" : 33}
# mutator.set_package_payload(package, value)
# print(package)
# print(mutator.get_package_payload(package))


# import pathlib
