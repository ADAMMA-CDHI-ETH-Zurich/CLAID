#pragma once
#include "dispatch/core/CLAID.hh"
#include "dispatch/proto/sensor_data_types.pb.h"

#import <CoreMotion/CoreMotion.h>


namespace claid
{
    class AccelerometerCollector : public claid::Module
    {

        private:
            Channel<AccelerationData> accelerometerDataChannel;
            CMMotionManager* motionManager;

            AccelerationData accelerationData;

        
            uint16_t samplingFrequency;

            void gatherAccelerometerSample();
            void postIfEnoughData();
        
        public:

            void initialize(const std::map<std::string, std::string>& propertiesMap);
            


       
        
    };
}