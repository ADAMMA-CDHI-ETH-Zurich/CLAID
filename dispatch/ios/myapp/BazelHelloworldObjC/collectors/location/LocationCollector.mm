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
        NSLog(@"Location 1");
        startLocationUpdates();       
    }


    

    void LocationCollector::startLocationUpdates()
    {
                NSLog(@"Location 2");

        if(isRunning)
        {
            return;
        }
        isRunning = true;

        
                NSLog(@"Location 3");

        if(this->currentPowerProfile.power_profile_type() == PowerProfileType::POWER_SAVING_MODE)
        {
            // Low power
            moduleWarning("Setting low power profile");
            [locationTracker setLowPowerProfile];
                    NSLog(@"Location 4");

        }
        else
        {
                    NSLog(@"Location 5");

            [locationTracker setUnrestrictedPowerProfile];
        }
                NSLog(@"Location 6");

        int samplingPeriod = (int) (this->currentPowerProfile.period());
        NSLog(@"Location 6");

        [locationTracker startLocationListener];
        NSLog(@"Location 8");


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