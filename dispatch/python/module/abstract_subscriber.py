from abc import ABC, abstractmethod

from dispatch.proto.claidservice_pb2 import DataPackage


class AbstractSubscriber(ABC):
    @abstractmethod
    def on_new_data(self, data: DataPackage):
        pass
