from typing import Type, TypeVar, Generic, Dict
from remote_function.abstract_future import AbstractFuture
from remote_function.future_unique_identifier import FutureUniqueIdentifier
from remote_function.future import Future
from remote_function.futures_table import FuturesTable

T = TypeVar("T")

class FutureHandler:
    def __init__(self):
        self.open_futures = FuturesTable()

    def register_new_future(self, return_data_type: Type[T]) -> Future:
        unique_identifier = FutureUniqueIdentifier.make_unique_identifier()
        future = Future(self.open_futures, unique_identifier, return_data_type)
        self.open_futures.add_future(future)
        return future

    def lookup_future(self, identifier: str) -> AbstractFuture:
        return self.open_futures.lookup_future(identifier)
