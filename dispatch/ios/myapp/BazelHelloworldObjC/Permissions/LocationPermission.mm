/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer
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

#import "LocationPermission.hh"

// Class should be rewritten with the same structure of MicrophonePermission, using methods of superclass Permission
@implementation LocationPermission


- (instancetype)init
{
    self = [super init];
    if (self) {
        _locationManager = [CLLocationManager new];
        _locationManager.delegate = self;
        _locationManager.allowsBackgroundLocationUpdates = YES;
        self.permissionTitle = @"This app needs location permissions to work";
        self.permissionBody = @"You need to allow location permissions. If you don't see the alert, you need to allow them in: \nSettings->APP_NAME->Location->Always";

        if(!self.isGranted)
        {
            [self displayBlockingAlertDialog];
            [self blockingRequest];
        }
    }
    return self;
}


- (bool) isGranted {
    CLAuthorizationStatus authStatus = [CLLocationManager authorizationStatus];
    if (authStatus == kCLAuthorizationStatusAuthorizedAlways)
    {
        return true;
    }
    return false;
}


- (void) blockingRequest {
    if (@available(iOS 14.0, *)) {
        [self locationManagerDidChangeAuthorization:_locationManager];
    }
    else {
        [self locationManager:_locationManager didChangeAuthorizationStatus:kCLAuthorizationStatusAuthorizedAlways];
    }
}

// iOS 14+ - From iOS 13.4 we need to perform incremental request, first we ask "whenInUse" and if it's granted we can ask for "always"
// We need to separate for iOS 13.4, because it still needs incremental but can't use manager.authorizationStatus.
- (void)locationManagerDidChangeAuthorization:(CLLocationManager *)manager {
    if (@available(iOS 14.0, *)) {
        switch (manager.authorizationStatus) {
            case kCLAuthorizationStatusNotDetermined:
                [manager requestWhenInUseAuthorization];
                [manager requestAlwaysAuthorization];
                break;
            case kCLAuthorizationStatusAuthorizedAlways:
                break;
            default:
                [manager requestAlwaysAuthorization];
                [self displayBlockingAlertDialog];
        }
    }
}


// iOS 14-
- (void)locationManager:(CLLocationManager *)manager didChangeAuthorizationStatus:(CLAuthorizationStatus)status
{
    switch (status)
    {
        case kCLAuthorizationStatusNotDetermined:
            [manager requestAlwaysAuthorization];
            break;
        case kCLAuthorizationStatusAuthorizedAlways:
            break;
        default:
            // Check for status on earlier version then iOS 14
            CLAuthorizationStatus authStatus = [CLLocationManager authorizationStatus];
            if (authStatus != kCLAuthorizationStatusAuthorizedAlways)
            {
                [manager requestAlwaysAuthorization];
                [self displayBlockingAlertDialog];
            }
    }
}

- (void)alertView:(UIAlertView *)alertView didDismissWithButtonIndex:(NSInteger)buttonIndex {
    if (buttonIndex == 0) {
        // We reload to check if we have permissions
        if (![self isGranted])
        {
            [self init];
            [[UIApplication sharedApplication] openURL:[NSURL URLWithString:UIApplicationOpenSettingsURLString]];
        }
    }
}

@end