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

import threading
import queue


class ThreadSafeChannel():
    def __init__(self, max_size=None):
        self.queue = queue.deque()
        self.lock = threading.Lock()
        self.condition = threading.Condition(self.lock)
        self.closed = False
        self.max_size = max_size

    def close(self):
        with self.lock:
            self.closed = True
            self.condition.notify_all()

    def is_closed(self):
        with self.lock:
            return self.closed

    def put(self, item):
        with self.lock:
            if self.closed:
                raise ValueError("put to closed channel")

            self.queue.append(item)

            if self.max_size is not None:
                if len(self.queue) > self.max_size:
                    self.queue.popleft()

            self.condition.notify()

    def get(self, wait=True):
        with self.lock:
            if wait:
                self.condition.wait_for(lambda: self.closed or len(self.queue) > 0)

            if not self.queue:
                return None

            return self.queue.popleft()

    def size(self):
        with self.lock:
            return len(self.queue)
