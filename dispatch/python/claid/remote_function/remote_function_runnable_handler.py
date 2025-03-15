import threading
from logger.logger import Logger
from typing import Callable, Dict, Any, List, Optional
from remote_function.remote_function_runnable import RemoteFunctionRunnable
from remote_function.remote_function_runnable_result import RemoteFunctionRunnableResult
from dispatch.proto.claidservice import *
import asyncio

async def queue_helper(queue, package):
    print("post helper")
    await queue.put(package)  # Await to ensure non-blocking behavior

class RemoteFunctionRunnableHandler:
    def __init__(self, entity_name: str, to_middleware_queue: asyncio.Queue, asyncio_loop):
        """
        :param entity_name: Name of the entity handling the functions.
        :param to_middleware_queue: A thread-safe queue for sending responses.
        """
        self.entity_name = entity_name
        self.to_middleware_queue = to_middleware_queue
        self.registered_runnables: Dict[str, RemoteFunctionRunnable] = {}
        self.asyncio_loop = asyncio_loop

    def add_runnable(self, function_name: str, runnable: RemoteFunctionRunnable) -> bool:
        """ Registers a runnable function if it's not already registered. """
        if function_name in self.registered_runnables:
            return False
        self.registered_runnables[function_name] = runnable
        return True

    def register_runnable(self, function_name: str, function: Callable, return_type: type, *parameter_types: type) -> bool:
        """ Registers a function dynamically if it exists and follows valid constraints. """
        parameter_list = list(parameter_types)

        if not callable(function):
            Logger.log_fatal(f"Failed to register function \"{function_name}\" in Module \"{self.entity_name}\". Provided function is not callable as it is of type {type(function)}")

        if function_name in self.registered_runnables:
            Logger.log_fatal(f"Failed to register function \"{function_name}\" in Module \"{self.entity_name}\". Function already registered.")
            return False
      
        if not RemoteFunctionRunnable.is_data_type_supported(return_type):
            Logger.log_fatal(f"Failed to register function \"{function_name}\" in Module \"{self.entity_name}\". Return type \"{return_type.__name__}\" is not supported.")
            return False

        for param in parameter_types:
            if not RemoteFunctionRunnable.is_data_type_supported(param):
                Logger.log_fatal(f"Failed to register function \"{function_name}\" in Module \"{self.entity_name}\". Parameter type \"{param.__name__}\" is not supported.")
                return False

        runnable = RemoteFunctionRunnable(function_name, function, return_type, *parameter_list)
        return self.add_runnable(function_name, runnable)

    def execute_remote_function_runnable(self, rpc_request: DataPackage) -> bool:
        """ Executes a registered remote function if it exists. """
        if not hasattr(rpc_request.control_val, "remote_function_request"):
            Logger.log_error("Failed to execute RPC request data package. Could not find definition of RemoteFunctionRequest.")
            response = RemoteFunctionRunnable.make_rpc_response_package(
                RemoteFunctionRunnableResult.make_failed_result(RemoteFunctionStatus.REMOTE_FUNCTION_REQUEST_INVALID),
                rpc_request, None
            )
            if response:
                self.to_middleware_queue.add(response)
            return False

        request = rpc_request.control_val.remote_function_request
        remote_function_identifier = request.remote_function_identifier
        function_name = remote_function_identifier.function_name

        if function_name not in self.registered_runnables:
            Logger.log_error(f"Failed to execute RPC request. Entity \"{self.entity_name}\" does not have a registered remote function called \"{function_name}\".")
            response = RemoteFunctionRunnable.make_rpc_response_package(
                RemoteFunctionRunnableResult.make_failed_result(RemoteFunctionStatus.FAILED_FUNCTION_NOT_FOUND_OR_FAILED_TO_EXECUTE),
                rpc_request, None
            )
            if response:
                self.to_middleware_queue.add(response)
            return False
        else:
            runnable = self.registered_runnables[function_name]
            print("Response")
            response = runnable.execute_remote_function_request(rpc_request)
            print("Response ", response)
            if response:
                print("sent")
                future = asyncio.run_coroutine_threadsafe(queue_helper(self.to_middleware_queue, response), self.asyncio_loop)  # ✅ Now passes a coroutine

                print("sent1")

        return True