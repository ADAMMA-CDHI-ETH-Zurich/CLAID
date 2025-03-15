from dispatch.proto.claidservice import *
from typing import Any, Optional

class RemoteFunctionRunnableResult:
    def __init__(self, return_value: Optional[Any], status: str):
        self.return_value = return_value
        self.status = status

    @classmethod
    def make_successful_result(cls, return_value: Any) -> 'RemoteFunctionRunnableResult':
        return cls(return_value, RemoteFunctionStatus.STATUS_OK)

    @classmethod
    def make_failed_result(cls, status: str) -> 'RemoteFunctionRunnableResult':
        return cls(None, status)

    def get_status(self) -> str:
        return self.status

    def get_return_value(self) -> Optional[Any]:
        return self.return_value