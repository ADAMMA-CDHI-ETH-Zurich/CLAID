from typing import Generic, TypeVar, Callable, Optional, Any
from dispatch.proto.claidservice import *
from remote_function.abstract_future import AbstractFuture
from module.type_mapping.type_mapping import TypeMapping

T = TypeVar('T')
class Future(AbstractFuture):
    def __init__(self, futures_table_in_handler, unique_identifier, return_type):
        super().__init__(futures_table_in_handler, unique_identifier)
        self.return_type = return_type
        self.callback_consumer = None
    
    def await_result(self) -> T:
        response_package = self.await_response()
        if response_package is None or not self.was_executed_successfully():
            return None
        return self.get_return_data(response_package)
    
    def get_return_data(self, response_package) -> Optional[T]:
        if response_package is None or self.return_type is None:
            return None
        
        if self.return_type == type(None):  # Handling void functions
            return None
        
        mutator = TypeMapping.get_mutator(self.return_type)
        return mutator.get_package_payload(response_package)
    
    def then(self, callback_consumer: Callable[[T], None]):
        self.callback_consumer = callback_consumer
        self.then_untyped(self.callback)
    
    async def callback(self, data):
        if self.callback_consumer is None:
            return
        result = self.get_return_data(data)
        await self.callback_consumer(result)
