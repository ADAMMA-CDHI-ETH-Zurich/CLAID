import threading
from typing import Generic, TypeVar, Callable, Optional
from dispatch.proto.claidservice import *


class AbstractFuture:
    def __init__(self, futures_table_in_handler=None, unique_identifier=None):
        self.finished = False
        self.response_package = None
        self.successful = False
        self.lock = threading.Lock()
        self.condition = threading.Condition(self.lock)
        self.futures_table_in_handler = futures_table_in_handler
        self.unique_identifier = unique_identifier
        self.untyped_callback = None
    
    def await_response(self):
        with self.condition:
            while not self.finished:
                self.condition.wait()
            
            if self.futures_table_in_handler:
                self.futures_table_in_handler.remove_future(self)
            
            return self.response_package if self.successful else None
    
    def then_untyped(self, untyped_callback: Callable[[DataPackage], None]):
        self.untyped_callback = untyped_callback
    
    def set_response(self, response_package):
        with self.condition:
            self.response_package = response_package
            self.successful = True
            if self.untyped_callback:
                self.untyped_callback(response_package)
            self.finished = True
            self.condition.notify_all()
    
    def set_failed(self):
        with self.condition:
            self.response_package = None
            self.successful = False
            if self.untyped_callback:
                self.untyped_callback(None)
            self.finished = True
            self.condition.notify_all()
    
    def get_unique_identifier(self):
        return self.unique_identifier
    
    def was_executed_successfully(self):
        return self.successful and self.finished