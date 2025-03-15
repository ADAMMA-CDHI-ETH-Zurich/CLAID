from typing import Callable, TypeVar, Any, List, Tuple
import logging

from module.type_mapping.type_mapping import TypeMapping
from remote_function.remote_function_runnable_result import RemoteFunctionRunnableResult
from dispatch.proto.claidservice import *
from logger.logger import Logger

ReturnType = TypeVar('ReturnType')
Parameter = TypeVar('Parameter')

class RemoteFunctionRunnable:
    def __init__(self, function_name: str, function: Callable[..., ReturnType], return_type, *param_examples: Any):
        """
        :param function_name: Name of the function to be executed.
        :param function: A callable function with a variable number of parameters.
        :param param_examples: Example parameters to infer types.
        """
        self.function_name = function_name
        self.function = function
        self.param_examples = param_examples
        self.return_type = return_type

        # Create mutator helpers based on parameter types
        print("Param examples: ", param_examples)
        self.mutator_helpers = [TypeMapping.get_mutator(param) for param in param_examples]

    def execute_remote_function_request(self, rpc_request: DataPackage) -> DataPackage:
        execution_request = rpc_request.control_val.remote_function_request
        remote_function_identifier = execution_request.remote_function_identifier

        parameter_payloads = execution_request.parameter_payloads
        if len(parameter_payloads) != len(self.mutator_helpers):
            Logger.log_fatal(f"Failed to execute RemoteFunctionRunnable {self.function_name}. "
                          f"Number of parameters do not match. Got {len(parameter_payloads)} parameters but expected {len(self.mutator_helpers)}")
            return None

        # Unpacking payloads into parameters
        parameters = self.unpack_parameters(parameter_payloads)

        # Execute function
        print("Executing function")
        result: ReturnType = self.function(*parameters)
        print("Executed function: ", result)
        # Create a successful function result
        remote_function_result = RemoteFunctionRunnableResult.make_successful_result(result)
        return self.make_rpc_response_package(remote_function_result, rpc_request)

    def unpack_parameters(self, payloads: List[Blob]) -> Tuple[Any, ...]:
        """
        Converts raw payloads into parameter values using type mutators.
        """
        parameters = []
        for index, (helper, param_example) in enumerate(zip(self.mutator_helpers, self.param_examples)):
            stub_package = DataPackage()
            stub_package.payload = payloads[index]
            value = helper.get_package_payload(stub_package)
            parameters.append(value)
        return tuple(parameters)

    def make_rpc_response_package(self, result: RemoteFunctionRunnableResult, rpc_request: DataPackage) -> DataPackage:
        """
        Creates a response package after executing the remote function.
        """
        print("Making response package")
        response_package = DataPackage()
        response_package.source_module = rpc_request.target_module
        response_package.target_module = rpc_request.source_module

        ctrl_package = ControlPackage()
        ctrl_package.ctrl_type = CtrlType.CTRL_REMOTE_FUNCTION_RESPONSE
        ctrl_package.runtime = rpc_request.control_val.runtime

        execution_request = rpc_request.control_val.remote_function_request
        ctrl_package.remote_function_return = self.make_remote_function_return(result, execution_request)
        response_package.control_val = ctrl_package
        self.set_return_package_payload(response_package, result)
        return response_package

    def make_remote_function_return(self, result: RemoteFunctionRunnableResult, execution_request: RemoteFunctionRequest) -> RemoteFunctionReturn:
        """
        Creates a return structure for a remote function call.
        """
        remote_function_return = RemoteFunctionReturn()
        remote_function_return.execution_status = result.get_status()
        remote_function_return.remote_function_identifier = execution_request.remote_function_identifier
        remote_function_return.remote_future_identifier = execution_request.remote_future_identifier
        return remote_function_return

    def set_return_package_payload(self, package: DataPackage, result: RemoteFunctionRunnableResult):
        """
        Sets the return value in the package if applicable.
        """
        return_value = result.get_return_value()
        if return_value is not None:
            mutator = TypeMapping.get_mutator(return_value)
            mutator.set_package_payload(package, return_value)



    @classmethod
    def is_data_type_supported(clz, data_type):
        try:
            mutator = TypeMapping.get_mutator(data_type)
            return True
        except Exception as e:
            return False
