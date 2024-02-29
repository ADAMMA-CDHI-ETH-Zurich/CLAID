
from module.scheduling.schedule_description import ScheduleDescription
from datetime import datetime

class ScheduleImmediatelyIndefinitely(ScheduleDescription):
    def __init__(self, execution_time: datetime):
        self.execution_time = execution_time

    @classmethod
    def startNow(cls):
        return cls(datetime.now())

    def does_runnable_have_to_be_repeated(self):
        return True

    def update_execution_time(self):
        # Does not exist for ScheduleOnce.
        # A ScheduledRunnable with ScheduleDescription
        # of type ScheduleOnce is not supposed to be repeated.
        self.execution_time = datetime.now()

    def get_execution_time(self):
        return self.execution_time
