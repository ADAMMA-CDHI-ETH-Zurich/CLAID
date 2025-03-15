import threading
import time

class FutureUniqueIdentifier:
    _mutex = threading.Lock()
    _current_id = 0

    def __init__(self, identifier: str):
        self._identifier = identifier

    @classmethod
    def make_unique_identifier(cls):
        with cls._mutex:
            id_copy = cls._current_id
            cls._current_id += 1
        
        identifier = f"CLAID_PY_{id_copy}_{int(time.time() * 1000)}"
        return cls(identifier)

    @classmethod
    def from_string(cls, identifier: str):
        return cls(identifier)

    def to_string(self) -> str:
        return self._identifier

    def __lt__(self, other):
        if not isinstance(other, FutureUniqueIdentifier):
            return NotImplemented
        return self._identifier < other._identifier
