from dispatch.proto.sensor_data_types_pb2 import HeartRateData


data = HeartRateData()

path = "/home/lastchance/Desktop/heart_rate_data_17.01.24_12.binary"
with open(path, mode="rb") as file:
    contents = file.read()

    if not data.ParseFromString(contents):
        print("Failed to load data")
        exit(0)
    print(data)
