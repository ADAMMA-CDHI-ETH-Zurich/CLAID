from dispatch.proto.sensor_data_types_pb2 import *
from dispatch.proto.claidservice_pb2 import * 
from dispatch.proto.claidservice_pb2_grpc import ClaidServiceStub

from logger.logger import Logger

import grpc

from timeit import time

from module.thread_safe_channel import ThreadSafeChannel

class ModuleDispatcher:
    def __init__(self, socket_path):
        self.socket_path = socket_path
  
        self.grpc_channel = grpc.insecure_channel(socket_path)

        self.stub = ClaidServiceStub(self.grpc_channel)

        # self.to_middleware_queue = io.Queue()
        self.__to_middleware_queue = ThreadSafeChannel()
        self.__from_middleware_queue = None
        self.__running = False


    def get_to_dispatcher_queue(self):
        return self.__to_middleware_queue
    
    def get_from_dispatcher_queue(self):
        return self.__from_middleware_queue

    def get_module_list(self, registered_module_classes, module_annotations):

      

        request = ModuleListRequest(
            runtime=Runtime.RUNTIME_PYTHON,
            supported_module_classes=registered_module_classes,
            module_annotations=module_annotations
        )
        Logger.log_info(f"Sending ModuleListRequest: {request}")

      

        Logger.log_info("Python Runtime: Calling getModuleList(...)")

        try:
            response =  self.stub.GetModuleList(request)

            if response is None:
                Logger.log_error(f"Error occurred in getModuleList() of PYTHON_RUNTIME: {response_observer.get_error_message()}")
                exit(0)
        except Exception as e:
            print(e)
            return False

        print(f"Response: {response}")
        return response

    def init_runtime(self, channel_example_packages):
        init_runtime_request = InitRuntimeRequest()

        print(channel_example_packages)
        for module_id in channel_example_packages:
            channel_packages = channel_example_packages[module_id]
            module_channels = InitRuntimeRequest.ModuleChannels(
                module_id=module_id,
                channel_packets=channel_packages
            )
            init_runtime_request.modules.append(module_channels)

        init_runtime_request.runtime = Runtime.RUNTIME_PYTHON

    
        Logger.log_info("Python Runtime: Calling initRuntime(...)")
        response =  self.stub.InitRuntime(init_runtime_request)


        if response is None:
            Logger.log_error(f"Error occurred in initRuntime() of Python: respone of InitRuntime is None")
            return False

        return True

    def make_control_runtime_ping(self):
        package = DataPackage()
        control_package = ControlPackage(
            ctrl_type=CtrlType.CTRL_RUNTIME_PING,
            runtime=Runtime.RUNTIME_PYTHON
        )

        package.control_val.CopyFrom(control_package)
        return package

  
    def to_middleware_queue_get(self):

        while self.__running:
            data = self.__to_middleware_queue.get()
            if not data is None:
                yield data
            elif not self.__running:
                return


    def send_receive_packages(self):

        self.__running = True
        self.__from_middleware_queue = self.stub.SendReceivePackages(self.to_middleware_queue_get())

        if self.__to_middleware_queue is None:
            Logger.log_error("Failed to initialize streaming to/from middleware: stub.sendReceivePackages return value is None.")
            return False

        self.waiting_for_ping_response = True
        ping_req = self.make_control_runtime_ping()

        Logger.log_info("Putting package")
        self.__to_middleware_queue.put(ping_req)

        ping_resp = self.await_ping_package()

        if(ping_resp.control_val.ctrl_type != CtrlType.CTRL_RUNTIME_PING):
            Logger.log_error(f"Sent ping package to middleware and expected ping package as response, but got: {ping_resp}")
            return False

        return True

    def await_ping_package(self):
        Logger.log_info("Waiting for ping package")
        for response in self.__from_middleware_queue:
            Logger.log_info(f"Got ping package {response}")
            return response
        
    def shutdown(self):
        self.__running = False
        self.__to_middleware_queue.put(None)
       
    # def on_middleware_stream_package_received(self, packet):
    #     Logger.log_info(f"Java Runtime received message from middleware: {packet}")

    #     if self.waiting_for_ping_response:
    #         if packet.control_val.ctrl_type != CtrlType.CTRL_RUNTIME_PING:
    #             Logger.log_error("Error in ModuleDispatcher: Sent CTRL_RUNTIME_PING to middleware and was waiting for a package with the same type as response,\n"
    #                              f"but got package of type {packet.control_val}")
    #             return

    #         if packet.control_val.runtime != Runtime.RUNTIME_JAVA:
    #             Logger.log_error("Error in ModuleDispatcher: Sent CTRL_RUNTIME_PING to middleware and received response, however response has wrong Runtime identifier.\n"
    #                              f"Expected RUNTIME_JAVA but got {packet.control_val.runtime}\n")
    #             return

    #         Logger.log_info("Java ModuleDispatcher successfully registered at the Middleware!")

    #         self.waiting_for_ping_response = False
    #         return

    #     Logger.log_info(f"Received package {packet.control_val}")
    #     self.in_consumer(packet)

    # def on_middleware_stream_error(self, throwable):
    #     Logger.log_error(f"Middleware stream closed! {throwable}")
    #     self.close_output_stream()

    # def on_middleware_stream_completed(self):
    #     Logger.log_error("Middleware stream completed!")
    #     self.close_output_stream()

    # def close_output_stream(self):
    #     self.out_stream.on_completed()
    #     self.out_stream = None

    # def post_package(self, packet):
    #     self.out_stream.on_next(packet)
    #     return False
