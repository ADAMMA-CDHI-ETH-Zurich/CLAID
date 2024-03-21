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

        int samplingPeriod;
        properties.getNumberProperty("samplingPeriod", samplingPeriod);

        if(properties.wasAnyPropertyUnknown())
        {
            moduleFatal(properties.getMissingPropertiesErrorString());
            return;
        }

        
        dispatch_async(dispatch_get_main_queue(), ^(void){
            locationPermission = [LocationPermission new];
        });
        
        while (true) {
            if (locationPermission.isGranted) {
                break;
            }
            [NSThread sleepForTimeInterval:3.0];
        }
        
        PowerProfile powerProfile;
        powerProfile.set_power_profile_type(PowerProfileType::UNRESTRICTED);
        powerProfile.set_period(samplingPeriod);
        this->currentPowerProfile = powerProfile;

        locationDataChannel = publish<LocationData>("LocationData");
        dispatch_async(dispatch_get_main_queue(), ^(void){
            locationTracker = [LocationTracker new];
        });

        moduleInfo("Initialized");

        startLocationUpdates();       
    }


    

    void LocationCollector::startLocationUpdates()
    {
        if(isRunning)
        {
            return;
        }
        isRunning = true;

        
        
        if(this->currentPowerProfile.power_profile_type() == PowerProfileType::POWER_SAVING_MODE)
        {
            // Low power
            [locationTracker setLowPowerProfile];
        }
        else
        {
            [locationTracker setUnrestrictedPowerProfile];
        }
        
        int samplingPeriod = (int) (this->currentPowerProfile.period());

        [locationTracker startLocationListener];


        moduleInfo("Starting location updates with samplingPeriod " + std::to_string(samplingPeriod));
        registerPeriodicFunction("SampleLocation", &LocationCollector::sampleLocation, this, Duration::milliseconds(samplingPeriod));
    }

    void LocationCollector::stopLocationUpdates()
    {
        if(!isRunning)
        {
            return;
        }

        isRunning = false;
        [locationTracker stopLocationListener];
        unregisterPeriodicFunction("SampleLocation");
    }


    void LocationCollector::restartLocationUpdates()
    {
        this->stopLocationUpdates();
        this->startLocationUpdates();
    }


    void LocationCollector::sampleLocation()
    {
        LocationSample locationSample;
        locationSample = locationTracker.getLastKnownLocation;

        LocationData locationData;
        *locationData.add_samples() = locationSample;
        locationDataChannel.post(locationData);
    }

    void LocationCollector::onPause()
    {
        moduleWarning("onPause called, unregistering from SensorManager");
        stopLocationUpdates();
    }

    void LocationCollector::onResume()
    {
        moduleWarning("onResume called, registering at SensorManager");
        startLocationUpdates();
    }

    void LocationCollector::onPowerProfileChanged(PowerProfile profile)
    {
        moduleWarning(absl::StrCat("onPowerProfileChanged called, using profile: ", messageToString(profile).c_str()));

    
        if(this->currentPowerProfile.power_profile_type() != profile.power_profile_type() ||
            this->currentPowerProfile.period() != profile.period())
        {
            moduleInfo("PowerProfile has changed, restarting");
            this->currentPowerProfile = profile;
            restartLocationUpdates();
        }

    }
}
REGISTER_MODULE(LocationCollector, claid::LocationCollector)