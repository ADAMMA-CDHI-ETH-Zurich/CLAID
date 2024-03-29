
#pragma once
#include "dispatch/proto/sensor_data_types.pb.h"

using claid::LocationSample;

#import <CoreLocation/CoreLocation.h>

@interface LocationTracker: NSObject <CLLocationManagerDelegate>

@property (strong) CLLocationManager *locationManager;
@property (nonatomic, assign) BOOL isRunning;
@property (nonatomic, assign) CLLocationDistance locationDistanceFilter;
@property (nonatomic, assign) CLLocationAccuracy locationAccuracy;

-(LocationSample) getLastKnownLocation;
-(void) setLowPowerProfile;
-(void) setUnrestrictedPowerProfile;
-(void) startLocationListener;
-(void) stopLocationListener;
@end
