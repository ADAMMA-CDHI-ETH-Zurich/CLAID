from dispatch.proto.sensor_data_types_pb2 import AccelerationData
from dispatch.proto.sensor_data_types_pb2 import HeartRateData
from dispatch.proto.sensor_data_types_pb2 import BatteryData



def load_acceleration_data(path):
    data = AccelerationData()

    with open(path, mode="rb") as file:
        contents = file.read()

        if not data.ParseFromString(contents):
            return None
        return data
            


def load_concat_acceleration_data_from_multiple_files(path_list):

    concatenated_data = AccelerationData()

    for path in path_list:
        data = load_acceleration_data(path)

        if(data is None):
            return None

        concatenated_data.MergeFrom(data)

    return concatenated_data



def load_heartrate_data(path):
    data = HeartRateData()

    with open(path, mode="rb") as file:
        contents = file.read()

        if not data.ParseFromString(contents):
            return None
        return data
            


def load_concat_heartrate_data_from_multiple_files(path_list):

    concatenated_data = HeartRateData()

    for path in path_list:
        data = load_heartrate_data(path)

        if(data is None):
            return None

        concatenated_data.MergeFrom(data)

    return concatenated_data




def load_battery_data(path):
    data = BatteryData()

    with open(path, mode="rb") as file:
        contents = file.read()

        if not data.ParseFromString(contents):
            return None
        return data
            


def load_concat_battery_data_from_multiple_files(path_list):

    concatenated_data = BatteryData()

    for path in path_list:
        data = load_battery_data(path)

        if(data is None):
            return None

        concatenated_data.MergeFrom(data)

    return concatenated_data
