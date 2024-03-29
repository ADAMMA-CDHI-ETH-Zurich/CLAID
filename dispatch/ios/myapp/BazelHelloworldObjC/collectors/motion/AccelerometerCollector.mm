/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
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

#import "AccelerometerCollector.hh"

namespace claid
{
    void AccelerometerCollector::initialize(Properties properties)
    {
        Logger::logInfo("Initializing");
        
        int samplingFrequency;
        properties.getNumberProperty("samplingFrequency", samplingFrequency);
       
        if(properties.wasAnyPropertyUnknown())
        {
            this->moduleFatal(properties.getMissingPropertiesErrorString());
            return;
        }

        
        PowerProfile powerProfile;
        powerProfile.set_power_profile_type(PowerProfileType::UNRESTRICTED);
        powerProfile.set_frequency(samplingFrequency);
        this->currentPowerProfile = powerProfile;
                                       
        accelerometerDataChannel = publish<AccelerationData>("AccelerationData");

        startSampling();
        Logger::logInfo("Starting business");
            
    }



    void AccelerometerCollector::gatherAccelerometerSample()
    {
        CMAccelerometerData* data = motionManager.accelerometerData;
        AccelerationSample& accelerationSample = *this->accelerationData.add_samples();      
        Time time = Time::now();

        Logger::logInfo("%f %f %f", data.acceleration.x, data.acceleration.y, data.acceleration.z);
        accelerationSample.set_acceleration_x(data.acceleration.x);
        accelerationSample.set_acceleration_y(data.acceleration.y);
        accelerationSample.set_acceleration_z(data.acceleration.z);
        accelerationSample.set_sensor_body_location("unknown/smartphone");
        accelerationSample.set_unix_timestamp_in_ms(time.toUnixTimestampMilliseconds());
        accelerationSample.set_effective_time_frame(time.strftime("%Y-%m-%0d'T'%H:%M:%S'Z'"));

        postIfEnoughData();       
    }



    void AccelerometerCollector::postIfEnoughData()
    {
        if(this->accelerationData.samples().size() >= this->currentPowerProfile.frequency())
        {
            Logger::logInfo("%s", messageToString(this->accelerationData).c_str());
            this->accelerometerDataChannel.post(this->accelerationData);
            this->accelerationData.Clear();
        }
    }







    void AccelerometerCollector::startSampling()
    {
        if(samplingIsRunning)
        {
            return;
        }
        moduleInfo("Starting sampling");

        motionManager = [CMMotionManager new];
        
        int samplingFrequency = this->currentPowerProfile.frequency();

        float samplingPeriod = 1.0 / (1.0 * samplingFrequency);
        
        // On iOS, we do not have to mind the PowerProfileType for the Accelerometer.
        // The accelerometer automatically throttles down based on what frequency we choose.

        if (motionManager.isAccelerometerAvailable)
        {
            motionManager.accelerometerUpdateInterval = samplingPeriod; 
            [motionManager startAccelerometerUpdates];
            
        }
        else
        {
            moduleError("Unable to start Accelerometer");
        }
        
        moduleInfo(std::string("Starting sampling at ") + std::to_string(samplingFrequency) + std::string("Hz (Period:") + std::to_string(samplingPeriod) + std::string(")"));
        samplingIsRunning = true;
        this->registerPeriodicFunction("AccelerometerSampling", &AccelerometerCollector::gatherAccelerometerSample, this, Duration::milliseconds(samplingPeriod * 1000));
    }

    void AccelerometerCollector::stopSampling()
    {
        if(!samplingIsRunning)
        {
            return;
        }
        moduleWarning("Stopping sampling");

        this->unregisterPeriodicFunction("AccelerometerSampling");
        [motionManager stopAccelerometerUpdates];
        motionManager = nil;

        samplingIsRunning = false;
        this->accelerationData.Clear();
    }

    void AccelerometerCollector::restartSampling()
    {
        stopSampling();
        startSampling();
    }

    void AccelerometerCollector::onPause()
    {
        moduleWarning("onPause called, unregistering from SensorManager");
        stopSampling();
    }

    void AccelerometerCollector::onResume()
    {
        moduleWarning("onResume called, registering at SensorManager");
        startSampling();
    }

    void AccelerometerCollector::onPowerProfileChanged(PowerProfile profile)
    {
        moduleWarning(absl::StrCat("onPowerProfileChanged called, using profile: ", messageToString(profile).c_str()));


        if(this->currentPowerProfile.power_profile_type() != profile.power_profile_type() ||
            this->currentPowerProfile.frequency() != profile.frequency())
        {
            moduleInfo("PowerProfile has changed, restarting");
            this->currentPowerProfile = profile;
            restartSampling();
        }
    

    }

}

REGISTER_MODULE(AccelerometerCollector, claid::AccelerometerCollector)


