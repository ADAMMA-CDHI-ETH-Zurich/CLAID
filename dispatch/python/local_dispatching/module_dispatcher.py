import grpc

from dispatch.proto.claidconfig_pb2 import *
from dispatch.proto.sensor_data_types_pb2 import *

import dispatch.proto.claid_service_pb2_grpc

from adamma.c4dhi.claid import ClaidServiceGrpc
from adamma.c4dhi.claid import DataPackage, ControlPackage, Runtime, CtrlType, InitRuntimeRequest, ModuleListRequest, Empty
from adamma.c4dhi.claid.Logger import Logger
from adamma.c4dhi.claid.Module import ModuleFactory
from adamma.c4dhi.claid.SynchronizedStreamObserver import SynchronizedStreamObserver

from io.grpc import Channel, Status

class ModuleDispatcher:
    def __init__(self, socket_path):
        self.socket_path = socket_path
        self.grpc_channel = None
        self.stub = None

        if socket_path is not None and socket_path.startswith("unix://") and self.is_android():
            uds_path = socket_path[len("unix://"):]
            self.grpc_channel = grpc.insecure_channel(uds_path)
        else:
            self.grpc_channel = grpc.insecure_channel(socket_path)

        self.stub = ClaidServiceGrpc.ClaidServiceStub(self.grpc_channel)

    def is_android(self):
        try:
            import android.os
            return True
        except ImportError:
            return False

    def get_module_list(self, registered_module_classes):
        request = ModuleListRequest(
            runtime=Runtime.RUNTIME_JAVA,
            supported_module_classes=registered_module_classes
        )
        Logger.log_info(f"Sending ModuleListRequest: {request}")

        response = None
        response_observer = SynchronizedStreamObserver()

        Logger.log_info("Java Runtime: Calling getModuleList(...)")
        self.stub.getModuleList(request, response_observer)

        response = response_observer.await_result()

        if response_observer.error_occurred():
            Logger.log_error(f"Error occurred in getModuleList() of JAVA_RUNTIME: {response_observer.get_error_message()}")
            exit(0)

        print(f"Response: {response}")
        return response

    def init_runtime(self, channel_example_packages):
        init_runtime_request = InitRuntimeRequest()

        for module_id, channel_packages in channel_example_packages.items():
            module_channels = InitRuntimeRequest.ModuleChannels(
                module_id=module_id,
                channel_packets=channel_packages
            )
            init_runtime_request.modules.append(module_channels)

        init_runtime_request.runtime = Runtime.RUNTIME_JAVA

        response = None
        response_observer = SynchronizedStreamObserver()

        Logger.log_info("Java Runtime: Calling initRuntime(...)")
        self.stub.initRuntime(init_runtime_request, response_observer)

        response = response_observer.await_result()

        if response_observer.error_occurred():
            Logger.log_error(f"Error occurred in initRuntime() of JAVA_RUNTIME: {response_observer.get_error_message()}")
            exit(0)

        return True

    def make_control_runtime_ping(self):
        builder = DataPackage()
        control_package_builder = ControlPackage(
            ctrl_type=CtrlType.CTRL_RUNTIME_PING,
            runtime=Runtime.RUNTIME_JAVA
        )

        builder.control_val.CopyFrom(control_package_builder)
        return builder

    def make_input_stream_observer(self, on_data, on_error, on_completed):
        def stream_observer(incoming_package):
            on_data(incoming_package)

        return grpc.stream.StreamObserver(
            on_next=stream_observer,
            on_error=on_error,
            on_completed=on_completed
        )

    def send_receive_packages(self, in_consumer):
        self.in_consumer = in_consumer

        if in_consumer is None:
            Logger.log_fatal("Invalid argument in ModuleDispatcher::sendReceivePackages. Provided consumer is null.")
            return False

        self.in_stream = self.make_input_stream_observer(
            lambda data_package: self.on_middleware_stream_package_received(data_package),
            lambda error: self.on_middleware_stream_error(error),
            lambda: self.on_middleware_stream_completed()
        )

        self.out_stream = self.stub.sendReceivePackages(self.in_stream)

        if self.out_stream is None:
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

    def on_middleware_stream_package_received(self, packet):
        Logger.log_info(f"Java Runtime received message from middleware: {packet}")

        if self.waiting_for_ping_response:
            if packet.control_val.ctrl_type != CtrlType.CTRL_RUNTIME_PING:
                Logger.log_error("Error in ModuleDispatcher: Sent CTRL_RUNTIME_PING to middleware and was waiting for a package with the same type as response,\n"
                                 f"but got package of type {packet.control_val}")
                return

            if packet.control_val.runtime != Runtime.RUNTIME_JAVA:
                Logger.log_error("Error in ModuleDispatcher: Sent CTRL_RUNTIME_PING to middleware and received response, however response has wrong Runtime identifier.\n"
                                 f"Expected RUNTIME_JAVA but got {packet.control_val.runtime}\n")
                return

            Logger.log_info("Java ModuleDispatcher successfully registered at the Middleware!")

            self.waiting_for_ping_response = False
            return

        Logger.log_info(f"Received package {packet.control_val}")
        self.in_consumer(packet)

    def on_middleware_stream_error(self, throwable):
        Logger.log_error(f"Middleware stream closed! {throwable}")
        self.close_output_stream()

    def on_middleware_stream_completed(self):
        Logger.log_error("Middleware stream completed!")
        self.close_output_stream()

    def close_output_stream(self):
        self.out_stream.on_completed()
        self.out_stream = None

    def post_package(self, packet):
        self.out_stream.on_next(packet)
        return False
