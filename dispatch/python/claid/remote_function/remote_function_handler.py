from dispatch.proto.claidservice import *
import asyncio
from remote_function.remote_function import RemoteFunction
from remote_function.future_handler import FutureHandler
from remote_function.future_unique_identifier import FutureUniqueIdentifier

from typing import Any
from logger.logger import Logger

class RemoteFunctionHandler:
    def __init__(self, to_middleware_queue: asyncio.Queue):
        self.futures_handler = FutureHandler()
        self.to_middleware_queue = to_middleware_queue

    def map_runtime_function(
        self, 
        runtime: Runtime, 
        function_name: str, 
        return_data_type_example: Any, 
        *parameter_type_examples: Any
    ) -> RemoteFunction:
        return RemoteFunction(
            self.futures_handler,
            self.to_middleware_queue,
            RemoteFunctionIdentifier(runtime=runtime, function_name=function_name),
            return_data_type_example,
            parameter_type_examples
        )

    def map_module_function(
        self, 
        target_module: str, 
        function_name: str, 
        return_data_type_example: Any, 
        *parameter_type_examples: Any
    ) -> RemoteFunction:
        return RemoteFunction(
            self.futures_handler,
            self.to_middleware_queue,
            RemoteFunctionIdentifier(module_id=target_module, function_name=function_name),
            return_data_type_example,
            parameter_type_examples
        )

    async def handle_response(self, remote_function_response: DataPackage):
        print("response ", remote_function_response)
        if not hasattr(remote_function_response.control_val, 'remote_function_return'):
            Logger.log_fatal("Failed to handle remote function response. Did not find RemoteFunctionReturn data")
            return

        Logger.log_info("RemoteFunctionHandler handling response 1")

        remote_function_return = remote_function_response.control_val.remote_function_return
        future_identifier = remote_function_return.remote_future_identifier
        unique_identifier = FutureUniqueIdentifier.from_string(future_identifier)
        Logger.log_info("RemoteFunctionHandler handling response 2")

        future = self.futures_handler.lookup_future(unique_identifier)
        if future is None:
            Logger.log_error(f"Cannot complete future, future with identifier {unique_identifier} is unknown.")
            return
        Logger.log_info("RemoteFunctionHandler handling response 3")

        if remote_function_return.execution_status != RemoteFunctionStatus.STATUS_OK:
            Logger.log_warning(f"Remote function failed. Future with identifier {future_identifier} failed with status {remote_function_return.execution_status}")
            future.set_failed()
            return
        Logger.log_info("RemoteFunctionHandler handling response 4")

        await future.set_response(remote_function_response)
        Logger.log_info("RemoteFunctionHandler handling response 5")