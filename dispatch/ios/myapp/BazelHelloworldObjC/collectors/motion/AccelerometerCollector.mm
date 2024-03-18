#import "AccelerometerCollector.hh"

namespace claid
{
    void AccelerometerCollector::initialize(const std::map<std::string, std::string>& propertiesMap)
    {
        Logger::logInfo("Initializing");
        PropertyHelper properties(propertiesMap);

        properties.getProperty("samplingFrequency", this->samplingFrequency);
       
        if(properties.wasAnyPropertyUnknown())
        {
            this->moduleFatal(properties.getMissingPropertiesErrorString());
            return;
        }

        motionManager = [CMMotionManager new];
        
        float samplingPeriod = 1.0 / (1.0 * this->samplingFrequency);
        
        if (motionManager.isAccelerometerAvailable)
        {
            motionManager.accelerometerUpdateInterval = samplingPeriod; // 50 Hz
            [motionManager startAccelerometerUpdates];
            
        }
        else
        {
            moduleError("Unable to start Accelerometer");
        }
        
        this->registerPeriodicFunction("GatherAccelerometerSamples", &AccelerometerCollector::gatherAccelerometerSample, this, Duration::milliseconds(samplingPeriod * 1000));
                                       
                                       
        accelerometerDataChannel = publish<AccelerationData>("AccelerationData");

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
        if(this->accelerationData.samples().size() >= this->samplingFrequency)
        {
            Logger::logInfo("%s", messageToString(this->accelerationData).c_str());
            this->accelerometerDataChannel.post(this->accelerationData);
            this->accelerationData.Clear();
        }
    }

}

REGISTER_MODULE(AccelerometerCollector, claid::AccelerometerCollector)


