from module.scheduling.schedule_description import ScheduleDescription
from datetime import datetime

class ScheduleRepeatedIntervall(ScheduleDescription):
    def __init__(self, start_time, interval_timedelta):
        self.execution_time = start_time
        self.interval_timedelta = interval_timedelta

    def does_runnable_have_to_be_repeated(self):
        return True

    def update_execution_time(self):
        self.execution_time += self.interval_timedelta
        # Uncomment the next line if you want to print the next schedule time
        # print(self.execution_time.strftime("Next schedule %H:%M:%S"))

    def get_execution_time(self):
        return self.execution_time
