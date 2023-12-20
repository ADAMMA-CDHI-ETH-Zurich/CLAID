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
