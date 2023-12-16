from module.module_factory import ModuleFactory
from logger.logger import Logger


class ModuleManager():

    def __init__(self, module_dispatcher, module_factory):
        self.__module_dispatcher = module_dispatcher
        self.__module_factory = module_factory
        self.__runningModules = dict()    
        self.__channel_subscriber_publisher = None

        self.__from_modules_stream = None
        self.__to_modules_stream = None
        
        self.__running = False

    def instantiate_module(self, module_id, module_class):

        if not self.__module_factory.is_module_class_registered(module_class):
            Logger.log_error(f"ModuleManager: Failed to instantiate Module of class \"{module_class}\" (id: {module_id}\")\n"
                  f"A Module with this class is not registered to the ModuleFactory.")
            return False

        Logger.log_info(f"Loaded Module id \"{module_id}\" (class: \"{module_class}\").")
        module = self.__module_factory.get_instance(module_class, module_id)
        self.__running_modules[module_id] = module
        return True

    def instantiate_modules(self, module_list):

        for descriptor in module_list.descriptors_list:
            module_id = descriptor.module_id
            module_class = descriptor.module_class

            if not self.instantiate_module(module_id, module_class):
                Logger.log_error(f"Failed to instantiate Module \"{module_id}\" (class: \"{module_class}\").\n"
                      f"The Module class was not registered to the ModuleFactory.")
                return False
        return True

    def initialize_modules(self, module_list, subscriber_publisher):
        for descriptor in module_list.descriptors_list:
            module_id = descriptor.module_id
            module_class = descriptor.module_class

            key = module_id
            if key not in self.running_modules:
                Logger.log_error(f"Failed to initialize Module \"{module_id}\" (class: \"{module_class}\").\n"
                      f"The Module was not loaded.")
                return False

            module = self.running_modules[key]

            Logger.log_info(f"Calling module.start() for Module \"{module.get_id()}\".")
            module.start(subscriber_publisher, descriptor.properties_map)
            Logger.log_info(f"Module \"{module.get_id()}\" has started.")

        return True

    def get_template_packages_of_modules(self):
        module_channels = {}
        for module_id in self.running_modules.keys():
            template_packages_for_module = self.subscriber_publisher.get_channel_template_packages_for_module(module_id)
            module_channels[module_id] = template_packages_for_module
        return module_channels

    def start(self):
        print(self.module_factory.get_registered_module_classes())
        module_list = self.dispatcher.get_module_list(self.module_factory.get_registered_module_classes())
        Logger.log_info(f"Received ModuleListResponse: {module_list}")
        if not self.instantiate_modules(module_list):
            Logger.log_fatal("ModuleDispatcher: Failed to instantiate Modules.")
            return False

        self.subscriber_publisher = ChannelSubscriberPublisher(self.from_modules_stream)

        if not self.initialize_modules(module_list, self.subscriber_publisher):
            Logger.log_fatal("Failed to initialize Modules.")
            return False

        example_packages_of_modules = self.get_template_packages_of_modules()
        if not self.dispatcher.init_runtime(example_packages_of_modules):
            Logger.log_fatal("Failed to initialize runtime.")
            return False

        if not self.dispatcher.send_receive_packages(lambda received_package: self.on_data_package_received(received_package)):
            Logger.log_fatal("Failed to set up input and output streams with middleware.")
            return False

        self.running = True
        self.read_from_modules_thread = Thread(target=self.read_from_modules)
        self.read_from_modules_thread.start()

        return True

    def split_host_module(self, addr):
        host_and_module = addr.split(":")

        if len(host_and_module) != 2:
            return None

        return host_and_module

    def on_data_package_received(self, data_package):
        if data_package.HasField('control_val'):
            self.handle_package_with_control_val(data_package)
            return

        channel_name = data_package.channel
        module_id = data_package.target_module

        Logger.log_info(f"ModuleManager received package with target for Module \"{module_id}\" on Channel \"{channel_name}\"")

        if self.subscriber_publisher is None:
            Logger.log_error("ModuleManager received DataPackage, however SubscriberPublisher is Null.")
            return

        if not self.subscriber_publisher.is_data_package_compatible_with_channel(data_package):
            Logger.log_info(f"ModuleManager received package with target for Module \"{module_id}\" on Channel \"{channel_name}\",\n"
                            f"however the data type of payload of the package did not match the data type of the Channel.\n"
                            f"Expected payload type \"{self.subscriber_publisher.get_payload_case_of_channel(channel_name).name}\" but got \"{data_package.payload_oneof_case.name}")
            return

        subscriber_list = self.subscriber_publisher.get_subscriber_instances_of_module(channel_name, module_id)

        if subscriber_list is None:
            Logger.log_info(f"ModuleManager received package with target for Module \"{module_id}\" on Channel \"{channel_name}\",\n"
                            f"however a Subscriber of the Module for this Channel was not found. The Module has no Subscriber for this Channel.")
        for subscriber in subscriber_list:
            subscriber.on_new_data(data_package)

    def handle_package_with_control_val(self, packet):
        if packet.control_val.ctrl_type == CtrlType.CTRL_CONNECTED_TO_REMOTE_SERVER:
            for module_id, module in self.running_modules.items():
                module.notify_connected_to_remote_server()
        elif packet.control_val.ctrl_type == CtrlType.CTRL_DISCONNECTED_FROM_REMOTE_SERVER:
            for module_id, module in self.running_modules.items():
                module.notify_disconnected_from_remote_server()
        else:
            Logger.log_warning(f"ModuleManager received package with unsupported control val {packet.control_val.ctrl_type}")

    def read_from_modules(self):
        while self.running:
            data_package = self.from_modules_channel.blocking_get()
            if data_package is not None:
                self.on_data_package_from_module(data_package)
            else:
                print("Read from modules null")

    def on_data_package_from_module(self, data_package):
        Logger.log_info(f"ModuleManager received local package from Module \"{data_package.source_module}\".")
        Logger.log_info(f"{data_package} ")
        self.dispatcher.post_package(data_package)
