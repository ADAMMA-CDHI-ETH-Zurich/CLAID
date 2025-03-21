import threading
from typing import Generic, TypeVar, Callable, Optional
from dispatch.proto.claidservice import *
import asyncio

class AbstractFuture:
    def __init__(self, futures_table_in_handler=None, unique_identifier=None):
        self._finished_event = asyncio.Event()
        self.response_package: Optional[DataPackage] = None
        self.successful = False
        self.futures_table_in_handler = futures_table_in_handler
        self.unique_identifier = unique_identifier
        self.untyped_callback: Optional[Callable[[Optional[DataPackage]], None]] = None

    async def await_response(self, timeout: Optional[float] = None):
        try:
            if timeout is not None:
                await asyncio.wait_for(self._finished_event.wait(), timeout=timeout)
            else:
                await self._finished_event.wait()
        except asyncio.TimeoutError:
            return None

        if self.futures_table_in_handler:
            self.futures_table_in_handler.remove_future(self)

        return self.response_package if self.successful else None

    def then_untyped(self, untyped_callback: Callable[[Optional[DataPackage]], None]):
        self.untyped_callback = untyped_callback

    async def set_response(self, response_package: DataPackage):
        self.response_package = response_package
        self.successful = True
        if self.untyped_callback:
            result = self.untyped_callback(response_package)
            if asyncio.iscoroutine(result):
                await result
        self._finished_event.set()

    async def set_failed(self):
        self.response_package = None
        self.successful = False
        if self.untyped_callback:
            result = self.untyped_callback(None)
            if asyncio.iscoroutine(result):
                await result
        self._finished_event.set()

    def get_unique_identifier(self):
        return self.unique_identifier

    def was_executed_successfully(self):
        return self.successful and self._finished_event.is_set()

    async def __call__(self, timeout: Optional[float] = None):
        return await self.await_response(timeout)