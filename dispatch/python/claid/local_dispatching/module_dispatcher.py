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

from dispatch.proto.claid import *
from dispatch.proto.claidservice import * 

from logger.logger import Logger

from timeit import time

from module.thread_safe_channel import ThreadSafeChannel
import asyncio
from grpclib.client import Channel
class ModuleDispatcher:
    def __init__(self, socket_path):
        self.socket_path = socket_path
  
        print(socket_path)
        self.grpc_channel = Channel(path="/tmp/claid_socket.grpc")

        self.stub = ClaidServiceStub(self.grpc_channel)

        # self.to_middleware_queue = io.Queue()
        self.__to_middleware_queue = asyncio.Queue()
        self.__from_middleware_queue = None
        self.__running = False

        self.__ping_package_ready = False
        self.__ping_package_acknowledged = False
        self.__ping_package = None


    def get_to_dispatcher_queue(self):
        return self.__to_middleware_queue
    
    def get_from_dispatcher_queue(self):
        return self.__from_middleware_queue

    async def get_module_list(self, registered_module_classes, module_annotations):

        request = ModuleListRequest(
            runtime=Runtime.PYTHON,
            supported_module_classes=registered_module_classes,
            module_annotations=module_annotations
        )
        Logger.log_info(f"Sending ModuleListRequest: {request}")

        Logger.log_info("Python Runtime: Calling getModuleList(...)")

        try:
            response = await self.stub.get_module_list(request)

            if response is None:
                Logger.log_error(f"Error occurred in getModuleList() of PYTHON_RUNTIME: {response_observer.get_error_message()}")
                exit(0)
        except Exception as e:
            print(e)
            return False

        print(f"Response: {response}")
        return response

    async def init_runtime(self, channel_example_packages):
        print("dispatcher 1")
        init_runtime_request = InitRuntimeRequest()
        print("dispatcher 2")

        print(channel_example_packages)
        for module_id in channel_example_packages:
            print("dispatcher 3")
            channel_packages = channel_example_packages[module_id]
            print("dispatcher 4")
            module_channels = InitRuntimeRequestModuleChannels(
                module_id=module_id,
                channel_packets=channel_packages
            )
            print("dispatcher 5")
            init_runtime_request.modules.append(module_channels)
        print("dispatcher 3")

        init_runtime_request.runtime = Runtime.PYTHON

        print("dispatcher 4")

        Logger.log_info("Python Runtime: Calling initRuntime(...)")
        response = await self.stub.init_runtime(init_runtime_request)


        if response is None:
            Logger.log_error(f"Error occurred in initRuntime() of Python: respone of InitRuntime is None")
            return False

        return True

    def make_control_runtime_ping(self):
        package = DataPackage()
        control_package = ControlPackage(
            ctrl_type=CtrlType.CTRL_RUNTIME_PING,
            runtime=Runtime.PYTHON
        )

        package.control_val = control_package
        return package

    
  
    async def to_middleware_queue_get(self):
        while self.__running:
            if self.__ping_package_acknowledged or not self.__ping_package_ready:
                print("Wait 1")
                data = await self.__to_middleware_queue.get()  # Await the async queue
                print("Wait 2")
                if data is not None:
                    print("Yield package", data)
                    yield data
                elif not self.__running:
                    return
            else:
                if self.__ping_package_ready:
                    yield self.__ping_package
                    self.__ping_package_ready = False

                    


    async def send_receive_packages(self):
        self.__ping_package_ready = False
        self.__ping_package_acknowledged = False
        self.__ping_package = None
        
        self.__running = True

        print("Send receive 1")
        self.__from_middleware_queue = self.stub.send_receive_packages(data_package_iterator=self.to_middleware_queue_get())
        print("Send receive 2")
        if self.__to_middleware_queue is None:
            
            Logger.log_error("Failed to initialize streaming to/from middleware: stub.sendReceivePackages return value is None.")
            return False

        self.waiting_for_ping_response = True

        Logger.log_info("Putting ping package")
        self.__ping_package = self.make_control_runtime_ping()
        self.__ping_package_acknowledged = False
        self.__ping_package_ready = True
        ping_resp = None
        print("waiting")

    
        ping_resp = await self.await_ping_package()
        print("Got ping")

        if(ping_resp.control_val.ctrl_type != CtrlType.CTRL_RUNTIME_PING):
            Logger.log_error(f"Sent ping package to middleware and expected ping package as response, but got: {ping_resp}")
            return False
        
        self.__ping_package_acknowledged = True
        self.__ping_package_ready = False
        self.__ping_package = None
        return True


    async def await_ping_package(self):
        Logger.log_info("Waiting for ping package")
        async for response in self.__from_middleware_queue:
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
