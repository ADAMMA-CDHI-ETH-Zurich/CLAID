class ChannelData:

    def __init__(self, data = None, timestamp=None, user_id = None):
        self.data = data
        self.timestamp = timestamp
        self.user_id = user_id

        self.valid = False

        if self.data is not None and self.timestamp is not None and self.user_id is not None:
            self.valid = True

    def get_data(self):
        return self.data

    def get_timestamp(self):
        return self.timestamp

    def get_user_id(self):
        return self.user_id

    def is_valid(self):
        return self.valid