from enum import Enum
from typing import Type, Generic

from module.publisher import Publisher
from module.subscriber import Subscriber
from module.channel_access_rights import ChannelAccessRights


class Channel():
    def __init__(self, parent, channel_id, publisher_or_subscriber):

        self.parent = parent
        self.channel_id = channel_id

        if isinstance(publisher_or_subscriber, Publisher):
            self.valid = True
            self.publisher = publisher_or_subscriber
            self.access_rights = ChannelAccessRights.WRITE
        elif isinstance(publisher_or_subscriber, Subscriber):
            self.valid = True
            self.subscriber = publisher_or_subscriber
            self.access_rights = ChannelAccessRights.READ
        else:
            self.valid = False

       

    @classmethod
    def new_invalid_channel(cls, channel_id: str) -> 'Channel':
        return cls(channel_id=channel_id)

    @classmethod
    def published_channel(cls, parent, channel_id: str, publisher: Publisher) -> 'Channel':
        return cls(channel_id=channel_id, access_rights=ChannelAccessRights.WRITE, parent=parent, publisher=publisher, valid=True)

    @classmethod
    def subscribed_channel(cls, parent, channel_id: str, subscriber: Subscriber) -> 'Channel':
        return cls(channel_id=channel_id, access_rights=ChannelAccessRights.READ, parent=parent, subscriber=subscriber, callback_registered=subscriber is not None, valid=True)

    def can_read(self) -> bool:
        return self.access_rights in {ChannelAccessRights.READ, ChannelAccessRights.READ_WRITE}

    def can_write(self) -> bool:
        return self.access_rights in {ChannelAccessRights.WRITE, ChannelAccessRights.READ_WRITE}

    def post(self, data) -> None:
        if not self.can_write():
            msg = f"Tried to post data to channel \"{self.channel_id}\", however\nit did not publish this channel before."
            self.parent.module_error(msg)
            return

        self.publisher.post(data)
