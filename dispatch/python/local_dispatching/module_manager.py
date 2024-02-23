from module.module_factory import ModuleFactory
from logger.logger import Logger
from module.channel_subscriber_publisher import ChannelSubscriberPublisher
from module.module_annotator import ModuleAnnotator
from module.thread_safe_channel import ThreadSafeChannel
from local_dispatching.module_injector import ModuleInjector
from dispatch.proto.claidservice_pb2 import * 

from threading import Thread
import time

class ModuleManager():

    def __init__(self, module_dispatcher, module_factory, modules_injection_storage_path = None):
        self.__module_dispatcher = module_dispatcher
        self.__module_factory = module_factory
        # In Python, it is possible to add Modules at runtime.

        if modules_injection_storage_path is not None:
            self.__module_injector = ModuleInjector(modules_injection_storage_path, self.__module_factory)
        self.__running_modules = dict()    

        # self.__from_module_dispatcher_queue = ThreadSafeChannel()
        self.__to_module_dispatcher_queue = module_dispatcher.get_to_dispatcher_queue()
        
        self.__channel_subscriber_publisher = ChannelSubscriberPublisher(self.__to_module_dispatcher_queue)

        self.__running = False
        self.__from_module_dispatcher_queue_closed = False

        self.__module__anotations_updated = False


    def instantiate_module(self, module_id, module_class):

        if not self.__module_factory.is_module_class_registered(module_class):
            Logger.log_error(f"ModuleManager: Failed to instantiate Module of class \"{module_class}\" (id: {module_id}\")\n"
                  f"A Module with this class is not registered to the ModuleFactory.")
            return False

        Logger.log_info(f"Loaded Module id \"{module_id}\" (class: \"{module_class}\").")
        module = self.__module_factory.get_instance(module_class, module_id)
        self.__running_modules[module_id] = module
        print(self.__running_modules)
        return True

    def instantiate_modules(self, module_list):
        print("bla ", module_list.descriptors)
        for descriptor in module_list.descriptors:
            module_id = descriptor.module_id
            module_class = descriptor.module_class

            if not self.instantiate_module(module_id, module_class):
                Logger.log_error(f"Failed to instantiate Module \"{module_id}\" (class: \"{module_class}\").\n"
                      f"The Module class was not registered to the ModuleFactory.")
                return False
        return True

    def initialize_modules(self, module_list, subscriber_publisher):
        for descriptor in module_list.descriptors:
            module_id = descriptor.module_id
            module_class = descriptor.module_class

            key = module_id
            if key not in self.__running_modules:
                Logger.log_error(f"Failed to initialize Module \"{module_id}\" (class: \"{module_class}\").\n"
                      f"The Module was not loaded.")
                return False

            module = self.__running_modules[key]

            Logger.log_info(f"Calling module.start() for Module \"{module.get_id()}\".")
            module.start(subscriber_publisher, descriptor.properties)
            Logger.log_info(f"Module \"{module.get_id()}\" has started.")

        return True

    def get_template_packages_of_modules(self):
        module_channels = {}
        for module_id in self.__running_modules.keys():
            template_packages_for_module = self.__channel_subscriber_publisher.get_channel_template_packages_for_module(module_id)
            module_channels[module_id] = template_packages_for_module
        return module_channels

    def start(self):
        print("ModuleManager1")
        print(self.__module_factory.get_registered_module_classes())
        print("ModuleManager2")

        registered_module_classes = self.__module_factory.get_registered_module_classes()
        module_annotations = dict()
        print("ModuleManager3")

        for registered_module_class in registered_module_classes:

            module_annotator = ModuleAnnotator(registered_module_class)
            has_annotate_module_function = self.__module_factory.get_module_annotation(registered_module_class, module_annotator)
       

            if(has_annotate_module_function):
                module_annotations[registered_module_class] = module_annotator.get_annotations()
        print("ModuleManager4")

        module_list =  self.__module_dispatcher.get_module_list(registered_module_classes, module_annotations)
        Logger.log_info(f"Received ModuleListResponse: {module_list}")
        if not self.instantiate_modules(module_list):
            Logger.log_fatal("ModuleDispatcher: Failed to instantiate Modules.")
            return False

        print("ModuleManager5")

        if not self.initialize_modules(module_list, self.__channel_subscriber_publisher):
            Logger.log_fatal("Failed to initialize Modules.")
            return False
        print("ModuleManager6")

        example_packages_of_modules = self.get_template_packages_of_modules()
        if not  self.__module_dispatcher.init_runtime(example_packages_of_modules):
            Logger.log_fatal("Failed to initialize runtime.")
            return False

        self.__from_module_dispatcher_queue_closed = False
        if not self.__module_dispatcher.send_receive_packages():
            Logger.log_fatal("Failed to set up input and output streams with middleware.")
            return False

        self.__from_module_dispatcher_queue = self.__module_dispatcher.get_from_dispatcher_queue()

        self.running = True
        self.read_from_module_dispatcher_thread = Thread(target=self.read_from_module_dispatcher)
        self.read_from_module_dispatcher_thread.start()

        return True
    
    def shutdown_modules(self):
        for module_name, module in self.__running_modules.items():
            print("Shutting down ", module_name)
            module.shutdown()
            print("Module has shutdown")
        
        self.__running_modules.clear()

    def stop(self):
         
        self.running = False
        self.__from_module_dispatcher_queue.cancel()
        self.read_from_module_dispatcher_thread.join()
        
        self.__channel_subscriber_publisher.reset()
    
    # def start_test(self):
    #     self.start()
    #     # io.ensure_future(self.start())

       
    #     # try:
    #     #     # Run the event loop
    #     #     io.get_event_loop().run_forever()
    #     # except KeyboardInterrupt:
    #     #     pass

    def split_host_module(self, addr):
        host_and_module = addr.split(":")

        if len(host_and_module) != 2:
            return None

        return host_and_module

    def on_data_package_received_from_module_dispatcher(self, data_package):
        if data_package.HasField('control_val'):
            self.handle_package_with_control_val(data_package)
            return

        channel_name = data_package.channel
        module_id = data_package.target_module

        Logger.log_info(f"ModuleManager received package with target for Module \"{module_id}\" on Channel \"{channel_name}\"")

        if self.__channel_subscriber_publisher is None:
            Logger.log_error("ModuleManager received DataPackage, however SubscriberPublisher is Null.")
            return

        if not self.__channel_subscriber_publisher.is_data_package_compatible_with_channel(data_package, module_id):
            payload_case = data_package.WhichOneof("payload_oneof")
            Logger.log_info(f"ModuleManager received package with target for Module \"{module_id}\" on Channel \"{channel_name}\",\n"
                            f"however the data type of payload of the package did not match the data type of the Channel.\n"
                            f"Expected payload type \"{self.__channel_subscriber_publisher.get_payload_case_of_channel(channel_name).name}\" but got \"{payload_case}")
            return

        subscriber_list = self.__channel_subscriber_publisher.get_subscriber_instances_of_module(channel_name, module_id)

        if subscriber_list is None:
            Logger.log_info(f"ModuleManager received package with target for Module \"{module_id}\" on Channel \"{channel_name}\",\n"
                            f"however a Subscriber of the Module for this Channel was not found. The Module has no Subscriber for this Channel.")
        for subscriber in subscriber_list:
            subscriber.on_new_data(data_package)

    def handle_package_with_control_val(self, packet):
        Logger.log_info("PYTHON HANDLE PACKAGE")
        if packet.control_val.ctrl_type == CtrlType.CTRL_CONNECTED_TO_REMOTE_SERVER:
            for module_id, module in self.__running_modules.items():
                module.notify_connected_to_remote_server()
        elif packet.control_val.ctrl_type == CtrlType.CTRL_DISCONNECTED_FROM_REMOTE_SERVER:
            for module_id, module in self.__running_modules.items():
                module.notify_disconnected_from_remote_server()
        elif packet.control_val.ctrl_type == CtrlType.CTRL_UNLOAD_MODULES:
            self.shutdown_modules()

            Logger.log_info("Python ModuleManager received CTRL_UNLOAD_MODULES")

            response = DataPackage()
            ctrl_package = response.control_val

            ctrl_package.ctrl_type = CtrlType.CTRL_UNLOAD_MODULES_DONE
            ctrl_package.runtime = Runtime.RUNTIME_PYTHON
            response.source_host = packet.target_host
            response.target_host = packet.source_host

            self.__to_module_dispatcher_queue.put(response)

            Logger.log_info("Unloading Modules done")
        elif packet.control_val.ctrl_type == CtrlType.CTRL_RESTART_RUNTIME:
            
            Logger.log_info("Python ModuleManager received CTRL_RESTART_RUNTIME")
            self.restart_thread = Thread(target=self.restart)
            self.restart_thread.start()

            self.__restart_control_package = packet
        elif packet.control_val.ctrl_type == CtrlType.CTRL_REQUEST_MODULE_ANNOTATIONS_RESPONSE:
            self.__module_annotations = packet.control_val.module_annotations
            self.__module__anotations_updated = True
        elif packet.control_val.ctrl_type == CtrlType.CTRL_ON_NEW_CONFIG_PAYLOAD_DATA:
            payload = packet.control_val.config_upload_payload
            
            self.inject_modules_from_config_upload_payload(payload)

        else:
            Logger.log_warning(f"ModuleManager received package with unsupported control val {packet.control_val.ctrl_type}")

    def read_from_module_dispatcher(self):
        while self.running:
            print("on read from module dispatcher")
            try:
                for data_package in self.__from_module_dispatcher_queue:
                    if data_package is not None:
                        self.on_data_package_received_from_module_dispatcher(data_package)
                    else:
                        pass
            except Exception as e:
                print(e)
        
        self.__from_module_dispatcher_queue_closed = True


    

    def restart(self):
        Logger.log_info("Stopping ModuleManager")
        self.stop()
        Logger.log_info("Stopping ModuleDispatcher")
        self.__module_dispatcher.shutdown()

        print("Waiting42")
        while not self.__from_module_dispatcher_queue_closed:
            print("Waiting")
        time.sleep(2)
        Logger.log_info("Restarting ModuleManager and ModuleDispatcher")
        self.start()

        response = DataPackage()
        ctrl_package = response.control_val

        ctrl_package.ctrl_type = CtrlType.CTRL_RESTART_RUNTIME_DONE
        ctrl_package.runtime = Runtime.RUNTIME_PYTHON
        response.source_host = self.__restart_control_package.target_host
        response.target_host = self.__restart_control_package.source_host

        self.__to_module_dispatcher_queue.put(response)

    def update_module_annotations(self):
        self.__module__anotations_updated = False
        
        package = DataPackage()
        ctrl_package = package.control_val

        ctrl_package.ctrl_type = CtrlType.CTRL_REQUEST_MODULE_ANNOTATIONS
        ctrl_package.runtime = Runtime.RUNTIME_PYTHON
   

        self.__to_module_dispatcher_queue.put(package)

    def are_module_annotations_updated(self):
        return self.__module__anotations_updated
    
    def get_module_annotations(self):
        return self.__module_annotations
    
    # dict[str, List]
    # dict(code, module_names)
    def inject_new_modules(self, module_descrption: dict):

        if self.__module_injector is None:
            return False

        for code_name in module_descrption:
            code, module_names = module_descrption[code_name]
            if not self.__module_injector.inject_claid_modules_from_python_file(code_name, code, module_names):
                return False
            
        
        self.__module_factory = self.__module_injector.rebuild_get_module_factory()
        self.__module_factory.print_registered_modules()

        return True


    def inject_modules_from_config_upload_payload(self, payload):
        