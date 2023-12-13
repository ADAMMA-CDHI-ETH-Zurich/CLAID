from typing import Type
from google.protobuf.message import Message
from dispatch.proto.claidservice_pb2 import DataPackage
from logger.logger import Logger

class Mutator:
    def __init__(self, setter=None, getter=None):
        self.setter = setter
        self.getter = getter

    def set_package_payload(self, packet: DataPackage, value):
        if self.setter:
            self.setter(packet, value)

    def get_package_payload(self, packet: DataPackage):
        if self.getter:
            return self.getter(packet)
        return None
