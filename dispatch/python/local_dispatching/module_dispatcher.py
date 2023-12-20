from dispatch.proto.claidconfig_pb2 import *
from dispatch.proto.sensor_data_types_pb2 import *
from dispatch.proto.claidservice_pb2 import * 
from dispatch.proto.claidservice_pb2_grpc import ClaidServiceStub

from logger.logger import Logger

import grpc
import asyncio

class ModuleDispatcher:
    def __init__(self, socket_path):
        self.socket_path = socket_path
  
        self.grpc_channel = grpc.aio.insecure_channel(socket_path)

        self.stub = ClaidServiceStub(self.grpc_channel)

        # self.to_dispatcher_queue = asyncio.Queue()
        self.from_dispatcher_queue = asyncio.Queue()


    async def get_module_list(self, registered_module_classes):
        request = ModuleListRequest(
            runtime=Runtime.RUNTIME_PYTHON,
            supported_module_classes=registered_module_classes
        )
        Logger.log_info(f"Sending ModuleListRequest: {request}")

      

        Logger.log_info("Java Runtime: Calling getModuleList(...)")
        response = await self.stub.getModuleList(request)

        if response is None:
            Logger.log_error(f"Error occurred in getModuleList() of PYTHON_RUNTIME: {response_observer.get_error_message()}")
            exit(0)

        print(f"Response: {response}")
        return response

    async def init_runtime(self, channel_example_packages):
        init_runtime_request = InitRuntimeRequest()

        for module_id, channel_packages in channel_example_packages:
            module_channels = InitRuntimeRequest.ModuleChannels(
                module_id=module_id,
                channel_packets=channel_packages
            )
            init_runtime_request.modules.append(module_channels)

        init_runtime_request.runtime = Runtime.RUNTIME_PYTHON

    
        Logger.log_info("Java Runtime: Calling initRuntime(...)")
        response = await self.stub.initRuntime(init_runtime_request, response_observer)


        if response is None:
            Logger.log_error(f"Error occurred in initRuntime() of JAVA_RUNTIME: {response_observer.get_error_message()}")
            return False

        return True

    def make_control_runtime_ping(self):
        package = DataPackage()
        control_package = ControlPackage(
            ctrl_type=CtrlType.CTRL_RUNTIME_PING,
            runtime=Runtime.RUNTIME_JAVA
        )

        package.control_val = control_package
        return package

  

    def send_receive_packages(self):

        self.to_dispatcher_queue = self.stub.sendReceivePackages(self.from_dispatcher_queue)

        if self.to_dispatcher_queue is None:
            Logger.log_error("Failed to initialize streaming to/from middleware: stub.sendReceivePackages return value is None.")
            return False

        self.waiting_for_ping_response = True
        ping_req = self.make_control_runtime_ping()
        self.out_stream.on_next(ping_req)

        self.await_ping_package()

        return True

    def await_ping_package(self):
        while self.waiting_for_ping_response:
            try:
                time.sleep(0.05)
            except InterruptedException as e:
                Logger.log_error(f"ModuleDispatcher error in awaitPingPackage: {e}")

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
