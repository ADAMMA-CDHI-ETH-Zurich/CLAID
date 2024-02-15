from dispatch.python.data_collection.load.load_sensor_data import * 
from data_collection.plot.plot_acceleration_data import * 
from data_collection.plot.plot_heart_rate_data import * 
from data_collection.plot.plot_battery_data import * 

paths = list()
for day in range(18, 21):
    for hour in range(0, 24):
        paths.append("/Users/planger/Documents/CLAID/CLAIDV2DataTest/{:02d}.01.24/acceleration_data_{:02d}.01.24_{:02d}.binary".format(day, day, hour))
        
data = load_concat_acceleration_data_from_multiple_files(paths)
plot_acceleration_data(data)
# paths = list()
# for day in range(18, 21):
#     for hour in range(0, 24):
#         paths.append("/Users/planger/Documents/CLAID/CLAIDV2DataTest/{:02d}.01.24/heart_rate_data_{:02d}.01.24_{:02d}.binary".format(day, day, hour))
        
# data = load_concat_heartrate_data_from_multiple_files(paths)
# plot_heartrate_data(data)

# paths = list()
# for day in range(18, 21):
#     for hour in range(0, 24):
#         paths.append("/Users/planger/Documents/CLAID/CLAIDV2DataTest/{:02d}.01.24/battery_data_{:02d}.01.24_{:02d}.binary".format(day, day, hour))
        
# data = load_concat_battery_data_from_multiple_files(paths)
# plot_battery_data(data)

exit(0)
