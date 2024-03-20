/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Francesco Feher
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*         http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
***************************************************************************/

#import "LocationTracker.hh"


@implementation LocationTracker

LocationSample locationSample;

- (instancetype)init
{
    self = [super init];
    if (self) {
        _locationManager = [CLLocationManager new];
        _locationManager.delegate = self;
        _locationManager.allowsBackgroundLocationUpdates = YES;
        _locationManager.distanceFilter = kCLDistanceFilterNone;
        _locationManager.desiredAccuracy = kCLLocationAccuracyBest;
        [self startLocationListener];
    }
    return self;
}


- (void)locationManager:(CLLocationManager *)manager didUpdateLocations:(NSArray *)locations {
    CLLocation* location = locations.lastObject;

    std::string sourceInformation = "Only available on iOS 15+";
    int16_t floor = (int16_t) location.floor.level;
    double hAccuracy = location.horizontalAccuracy;
    double vAccuracy = location.verticalAccuracy;
    double speed = location.speed;
    double altitude = location.altitude;
    double latitude = location.coordinate.latitude;
    double longitude = static_cast<double>(location.coordinate.longitude);
    int64_t timestamp = (int64_t) [location.timestamp timeIntervalSince1970];
    double elapsedRealtimeSeconds = [[NSProcessInfo processInfo] systemUptime];
    double course = location.course;
    
    // TODO: Check why sourceInformation returns object and not value
    if (@available(iOS 15.0, *)) {
        NSString *NSSourceInformation = [NSString stringWithFormat:@"%@", location.sourceInformation];
        sourceInformation = [NSSourceInformation UTF8String];
    }
    
    NSLog(@"floor.level: %d", location.floor.level);
    NSLog(@"horizontalAccuracy: %f", location.horizontalAccuracy);
    NSLog(@"verticalAccuracy: %f", location.verticalAccuracy);
    NSLog(@"speed: %f", location.speed);
    NSLog(@"altitude: %f", location.altitude);
    NSLog(@"latitude: %f", location.coordinate.latitude);
    NSLog(@"longitude: %f", location.coordinate.longitude);
    NSLog(@"timestamp: %lld", [location.timestamp timeIntervalSince1970]);
    NSLog(@"elapsedRealtimeSeconds: %f", [[NSProcessInfo processInfo] systemUptime]);
    NSLog(@"course: %f", location.course);
    
    std::cout<<std::setprecision(12);
    std::cout<<std::endl<<"---Floor: "<<floor<<" HAccuracy: "<<hAccuracy<<" VAccuracy: "<<vAccuracy<<" Speed: "<<speed<<" Altitude: "<<
    altitude<<" Latitude: "<<latitude<<" Longitude: "<<longitude<<" Timestamp: "<<timestamp<<" ElapsedNano: "<<elapsedRealtimeSeconds<<
    " Course: "<<course<<" Source Info: "<<sourceInformation<<"---"<<std::endl<<std::endl;
    
    time_t seconds = (time_t)((int64_t) timestamp);
    std::string dateString = std::ctime(&seconds);
    std::cout << "timestamp formatted: "<<dateString<< std::endl;
  
    
    locationSample.set_floor(floor);
    locationSample.set_haccuracy(hAccuracy);
    locationSample.set_vaccuracy(vAccuracy);
    locationSample.set_speed(speed);
    locationSample.set_altitude(altitude);
    locationSample.set_latitude(latitude);
    locationSample.set_longitude(longitude);
    locationSample.set_timestamp(timestamp);
    locationSample.set_elapsedrealtimeseconds(elapsedRealtimeSeconds);
    locationSample.set_bearing(course);
    locationSample.set_provider(sourceInformation);

}


- (void)startLocationListener {
    [_locationManager startMonitoringSignificantLocationChanges];
    [_locationManager startUpdatingLocation];
}

- (void)stopLocationListener {
    dispatch_async(dispatch_get_main_queue(), ^(void){
        [_locationManager stopMonitoringSignificantLocationChanges];
        [_locationManager stopUpdatingLocation];
    });
}

- (LocationSample)getLastKnownLocation {
    [self stopLocationListener];
    return locationSample;
}


@end