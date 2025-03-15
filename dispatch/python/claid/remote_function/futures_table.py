import threading

class FuturesTable:
    def __init__(self):
        self.futures = {}
        self.lock = threading.Lock()

    def add_future(self, future):
        with self.lock:
            self.futures[future.get_unique_identifier().to_string()] = future

    def remove_future(self, future):
        future_identifier = future.get_unique_identifier().to_string()

        with self.lock:
            if future_identifier in self.futures:
                if self.futures[future_identifier] is future:
                    del self.futures[future_identifier]
                    return True
        return False

    def lookup_future(self, unique_identifier):
        with self.lock:
            return self.futures.get(unique_identifier.to_string(), None)
