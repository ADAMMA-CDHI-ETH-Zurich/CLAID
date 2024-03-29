###########################################################################
# Copyright (C) 2023 ETH Zurich
# CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
# Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
# Centre for Digital Health Interventions (c4dhi.org)
# 
# Authors: Patrick Langer
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#         http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################

from module.scheduling.schedule_description import ScheduleDescription

from module.scheduling.scheduled_runnable import ScheduledRunnable


# A runnable wrapper to schedule runnables from outside of CLAID.
# Only works with the CLAID main_thread_queue.
class ExternalScheduledRunnable(ScheduledRunnable):
    def __init__(self, runnable = None, schedule = None):
        self.runnable = runnable
        self.schedule = schedule

    def set_runnable(self, runnable):
        self.runnable = runnable

    def set_schedule(self, schedule: ScheduleDescription):
        self.schedule = schedule

