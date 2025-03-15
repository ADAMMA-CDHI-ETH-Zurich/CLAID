from typing import Any, List, Optional, Tuple
from module.type_mapping.type_mapping import TypeMapping
from remote_function.future_handler import FutureHandler
from remote_function.future import Future

import asyncio

from dispatch.proto.claidservice import *

class RemoteFunction:
    def __init__(
        self,
        futures_handler: FutureHandler,
        to_middleware_queue: asyncio.Queue,
        remote_function_identifier: RemoteFunctionIdentifier,
        return_type_example: Any,
        parameter_type_examples: Optional[Tuple[Any, ...]] = None,
    ):
        self.futures_handler = futures_handler
        self.to_middleware_queue = to_middleware_queue
        self.remote_function_identifier = remote_function_identifier 
        self.return_type_example = return_type_example 
        self.parameter_type_examples = parameter_type_examples 

        self.valid = futures_handler != None and to_middleware_queue != None and remote_function_identifier != None
        
        self.mutator_helpers = []
        if self.parameter_type_examples:
            for param in self.parameter_type_examples:
                mutator = TypeMapping.get_mutator(param)
                self.mutator_helpers.append(mutator)

    @staticmethod
    def invalid_remote_function() -> "RemoteFunction":
        return RemoteFunction()

    def get_num_params(self) -> int:
        return len(self.mutator_helpers)

    def execute(self, *params: Any) -> Future:
        if not self.valid:
            raise Exception("Failed to execute RemoteFunction. Function is not valid.")

        param_count = self.get_num_params()
        if param_count != len(self.mutator_helpers):
            raise Exception(
                f"Number of parameters do not match. Expected {len(self.mutator_helpers)}, but got {param_count}"
            )

        if not self.futures_handler:
            raise Exception("Cannot execute RemoteFunction. FuturesHandler is null.")
        
        if self.return_type_example is None:
            raise Exception("Cannot execute RemoteFunction. The dataTypeExample is null.")

        future = self.futures_handler.register_new_future(self.return_type_example)
        data_package = DataPackage()
        data_package.control_val = ControlPackage()
        data_package.control_val.runtime = Runtime.PYTHON
        data_package.control_val.ctrl_type = CtrlType.CTRL_REMOTE_FUNCTION_REQUEST
        
        request = self.make_remote_function_request(future.get_unique_identifier(), *params)
        data_package.control_val.remote_function_request = request
        
        print(self.remote_function_identifier, hasattr(self.remote_function_identifier, "module_id"))
        if hasattr(self.remote_function_identifier, "module_id"):
            data_package.target_module = self.remote_function_identifier.module_id

        if not self.to_middleware_queue:
            raise Exception("Cannot execute RemoteFunction. Queue toMiddlewareQueue is null.")
        
        print("Put nowait")
        self.to_middleware_queue.put_nowait(data_package)
        return future

    def __call__(self, *params: Any) -> Optional[Any]:
        future = self.execute(*params)
        if future is None:
            return None
        return future.await_result()

    def make_remote_function_request(self, future_identifier: str, *params: Any) -> dict:
        request = RemoteFunctionRequest()
        request.remote_function_identifier = self.remote_function_identifier
        request.remote_future_identifier = future_identifier.to_string()
        
        parameter_payloads = []
        for index, param in enumerate(params):
            stub_package = DataPackage()
            self.mutator_helpers[index].set_package_payload(stub_package, param)
            parameter_payloads.append(stub_package.payload)
        
        request.parameter_payloads = parameter_payloads
        
        return request
