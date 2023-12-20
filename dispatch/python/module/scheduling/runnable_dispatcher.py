import threading
import time
from typing import List
from logger.logger import Logger
from datetime import datetime

class RunnableDispatcher:
    def __init__(self):
        self.scheduled_runnables = []
        self.mutex = threading.Lock()
        self.condition_variable = threading.Condition(self.mutex)
        self.reschedule_required = False
        self.stopped = True
        self.thread = None

    def get_wait_duration_until_next_runnable_is_due(self):
        with self.mutex:
            if not self.scheduled_runnables:
                # Wait forever.
                return float('inf')

            microseconds_until_next_runnable_is_due = (
                self.scheduled_runnables[0].schedule.get_execution_time() - datetime.now()
            )

            # Make sure we do not return a negative duration.
            return max(0, microseconds_until_next_runnable_is_due / 1000)

    def wait_until_runnable_is_due_or_reschedule_is_required(self):
        with self.mutex:
            # If there is no runnable added, this will be infinity.
            # Hence, we will wait forever and wake up if a reschedule is required.
            wait_time = self.get_wait_duration_until_next_runnable_is_due() / 1000

            # The return value of wait_for can be used to determine whether the wait exited because time passed (False),
            # or because the predicate (self.reschedule_required or self.stopped) evaluates to True (True).
            # However, we are not interested in distinguishing the two cases.
            # In any case, when we wake up, we see if we need to execute anything.
            # wait_for will atomically release the mutex and sleep, and will atomically lock the mutex after waiting.
            with self.condition_variable:
                self.condition_variable.wait_for(
                    lambda: self.reschedule_required or self.stopped, wait_time
                )

    def process_runnable(self, scheduled_runnable):
        if self.stopped:
            return

        if scheduled_runnable.runnable.is_valid():
            if scheduled_runnable.runnable.catch_exceptions:
                try:
                    Logger.log_info(
                        "Running runnable! Remaining runnables: %d",
                        len(self.scheduled_runnables),
                    )
                    scheduled_runnable.runnable.run()
                except Exception as e:
                    scheduled_runnable.runnable.set_exception(str(e))
            else:
                scheduled_runnable.runnable.run()

            scheduled_runnable.runnable.was_executed = True

            if scheduled_runnable.runnable.stop_dispatcher_after_this_runnable:
                # No more runnables will be executed after this one!
                self.stop()
                print("STOPPED DISPATCHER!")
                return

            if scheduled_runnable.schedule.does_runnable_have_to_be_repeated():
                with self.mutex:
                    planned_execution_time = scheduled_runnable.schedule.get_execution_time()
                    scheduled_runnable.schedule.update_execution_time()

                    planned_execution_time_str = Time.strftime(
                        "%d.%m.%y %H:%M:%S", time.localtime(planned_execution_time / 1000)
                    )
                    current_execution_str = Time.strftime(
                        "%d.%m.%y %H:%M:%S", time.localtime(datetime.now() / 1000)
                    )

                    new_execution_time = scheduled_runnable.schedule.get_execution_time()
                    next_execution_str = Time.strftime(
                        "%d.%m.%y %H:%M:%S", time.localtime(new_execution_time / 1000)
                    )

                    Logger.log_info(
                        "Runnable, scheduled for execution at %s, has been executed at %s, scheduling next execution for %s",
                        planned_execution_time_str,
                        current_execution_str,
                        next_execution_str,
                    )

                    # Reinsert the runnable with new scheduled execution time.
                    # Note, that the runnable was removed from scheduled_runnables in the get_and_remove_due_runnables() function.
                    self.scheduled_runnables.append(scheduled_runnable)
                    self.scheduled_runnables.sort(
                        key=lambda x: x.schedule.get_execution_time()
                    )

    def process_runnables(self, runnables):
        for idx, runnable in enumerate(runnables):
            Logger.log_info("Processing runnable %d", idx)
            self.process_runnable(runnable)

    def get_and_remove_due_runnables(self):
        runnables = []
        now = datetime.now()

        with self.mutex:
            while self.scheduled_runnables:
                due_runnable = self.scheduled_runnables[0]
                if now >= due_runnable.schedule.get_execution_time():
                    runnables.append(due_runnable)
                    self.scheduled_runnables.pop(0)
                else:
                    break

        if runnables:
            Logger.log_info(
                "Found %d due runnables out of %d runnables in total: ",
                len(runnables),
                len(self.scheduled_runnables) + len(runnables),
            )
            Logger.log_info(
                "Removed %d due runnables, number of remaining runnables is %d ",
                len(runnables),
                len(self.scheduled_runnables),
            )

        return runnables

    def run_scheduling(self):
        Logger.log_info("Running scheduling")
        while not self.stopped:
            due_runnables = self.get_and_remove_due_runnables()

            while due_runnables:
                self.process_runnables(due_runnables)
                due_runnables = self.get_and_remove_due_runnables()

            with self.mutex:
                self.reschedule_required = False

            self.wait_until_runnable_is_due_or_reschedule_is_required()

        Logger.log_info("RunnableDispatcher shutdown.")

    def start(self):
        with self.mutex:
            if not self.stopped:
                return False

            self.stopped = False
            self.thread = threading.Thread(target=self.run_scheduling)
            self.thread.start()
            return True

    def stop(self):
        with self.mutex:
            if self.stopped:
                return False

            self.stopped = True

        self.thread.join()
        return True

    def add_runnable(self, scheduled_runnable):
        with self.mutex:
            self.scheduled_runnables.append(scheduled_runnable)
            self.scheduled_runnables.sort(
                key=lambda x: x.schedule.get_execution_time()
            )
            Logger.log_info(
                "Added runnable to RunnableDispatcher, total runnables now: %d",
                len(self.scheduled_runnables),
            )
            # This will lead to a wake up, so we can reschedule.
           