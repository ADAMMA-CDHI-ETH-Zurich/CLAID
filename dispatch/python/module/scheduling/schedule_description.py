from abc import ABC, abstractmethod
from datetime import datetime

class ScheduleDescription(ABC):
    @abstractmethod
    def does_runnable_have_to_be_repeated(self):
        pass

    @abstractmethod
    def update_execution_time(self):
        pass

    @abstractmethod
    def get_execution_time(self) -> datetime:
        pass
