from module.scheduling.schedule_description import ScheduleDescription

class ScheduledRunnable:
    def __init__(self, runnable = None, schedule: ScheduleDescription = None):
        self.runnable = runnable
        self.schedule = schedule

    def set_runnable(self, runnable):
        self.runnable = runnable

    def set_schedule(self, schedule: ScheduleDescription):
        self.schedule = schedule

