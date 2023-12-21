from datetime import datetime
from module.type_mapping.type_mapping import TypeMapping
from module.type_mapping.mutator import Mutator

class Publisher:
    def __init__(self, channel_data_type_example_package, module_id: str, channel_name: str, to_module_manager_queue):
        self.module_id = module_id
        self.channel_name = channel_name
        self.to_module_manager_queue = to_module_manager_queue
        self.mutator = TypeMapping.get_mutator(channel_data_type_example_package)

    def post(self, data):
        package = DataPackage()
        package.source_module = self.module_id
        package.channel = self.channel_name
        package.unix_timestamp_ms = int(datetime.now().timestamp() * 1000)

        self.mutator.set_package_payload(package, data)

        self.to_module_manager_queue.put(package)