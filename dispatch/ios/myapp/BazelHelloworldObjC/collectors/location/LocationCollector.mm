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

#import "LocationCollector.hh"
#import <CoreLocation/CoreLocation.h>

namespace claid
{
    void LocationCollector::initialize(Properties properties)
    {
        moduleInfo("LocationCollector init");
        
        dispatch_async(dispatch_get_main_queue(), ^(void){
            locationPermission = [LocationPermission new];
        });
        
        while (true) {
            if (locationPermission.isGranted) {
                break;
            }
            [NSThread sleepForTimeInterval:3.0];
        }
        
        dispatch_async(dispatch_get_main_queue(), ^(void){
            locationTracker = [LocationTracker new];
        });

        locationDataChannel = publish<LocationData>("LocationData");
        

        moduleInfo("Initialized");
            
    }


    void LocationCollector::postLocationData()
    {
        LocationSample locationSample;
        locationSample = locationTracker.getLastKnownLocation;

        LocationData locationData;
        *locationData.add_samples() = locationSample;
        locationDataChannel.post(locationData);
    }


    // void LocationCollector::onLocationDataRequested(claid::ChannelData<claid::Request> data)
    // {
    //     claid::Request request = data->value();
        
    //     dispatch_async(dispatch_get_main_queue(), ^(void){
    //         locationTracker = [LocationTracker new];
    //     });
        
    //     if(request.dataIdentifier == "LocationData")
    //     {
    //         std::cout<<"LocationData requested"<<std::endl;
    //         postLocationData();
    //     }
        
    // }
}
REGISTER_MODULE(LocationCollector, claid::LocationCollector)