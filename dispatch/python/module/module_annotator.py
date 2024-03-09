from dispatch.proto.claidservice_pb2 import ModuleAnnotation, DataPackage, PropertyHint, PropertyType
from module.type_mapping.type_mapping import TypeMapping

class ModuleAnnotator:
    def __init__(self, module_type: str):
        self.annotation = ModuleAnnotation()
        self.module_type = module_type

    def prepare_example_package(self, data_type_example, module_id: str, channel_name: str, is_publisher: bool) -> DataPackage:
        data_package = DataPackage()

        if is_publisher:
            data_package.source_module = module_id
        else:
            data_package.target_module = module_id

        print(type(channel_name))
        data_package.channel = channel_name

        mutator = TypeMapping.get_mutator(example_instance=data_type_example)
        mutator.set_package_payload(data_package, data_type_example.__class__())

        return data_package

    def set_module_description(self, module_description: str):
        self.annotation.module_description = module_description

    def set_module_category(self, module_category: str):
        self.annotation.module_category = module_category
    
    def make_default_property(self):
        property_hint = PropertyHint()
        property_hint.property_type = PropertyType.PROPERTY_TYPE_DEFAULT
        return property_hint

    def make_enum_property(self, enum_values : list):
        property_hint = PropertyHint
        property_hint.property_type = PropertyType.PROPERTY_TYPE_ENUM
        property_hint.enum_values = enum_values
        return property_hint
    

    def make_integer_property(self, min, max):
        property_hint = PropertyHint
        property_hint.property_type = PropertyType.PROPERTY_TYPE_INT
        property_hint.property_type_int_min = min
        property_hint.property_type_int_max = max
        
        return property_hint


    def make_path_property(self):
        property_hint = PropertyHint
        property_hint.property_type = PropertyType.PROPERTY_TYPE_PATH
        
        return property_hint

    def describe_property(self, property_name: str, property_description: str, property_hint : PropertyHint = None):
        self.annotation.properties.append(property_name)
        self.annotation.property_descriptions.append(property_description)

        if property_hint == None:
            property_hint = self.make_default_property()
        
        self.annotation.property_hints.append(property_hint)


    def describe_publish_channel(self, channel_name: str, data_type_example, channel_description: str):
        example_package = self.prepare_example_package(data_type_example, self.module_type, channel_name, True)
        self.annotation.channel_definition.append(example_package)
        self.annotation.channel_description.append(channel_description)

    def describe_subscribe_channel(self, channel_name: str, data_type_example, channel_description: str):
        example_package = self.prepare_example_package(data_type_example, self.module_type, channel_name, False)
        self.annotation.channel_definition.append(example_package)
        self.annotation.channel_description.append(channel_description)

    def make_injectable(self):
        self.annotation.is_injectable = True

    def add_dependency(self, dependency: str):
        self.annotation.file_dependencies.append(dependency)

    def get_annotations(self) -> ModuleAnnotation:
        return self.annotation
