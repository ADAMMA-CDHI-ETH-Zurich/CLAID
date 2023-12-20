from module.scheduling.schedule_description import ScheduleDescription

class ScheduledRunnable:
    def __init__(self, schedule: ScheduleDescription):
        self.schedule = schedule
        self.valid = True

    def run(self):
        pass

    def get_schedule(self):
        return self.schedule

    def is_valid(self):
        return self.valid

    def invalidate(self):
        self.valid = False
