#pragma once
#include "dispatch/core/CLAID.hh"
#include "dispatch/proto/sensor_data_types.pb.h"

using namespace claidservice;

#import <CoreMotion/CoreMotion.h>


namespace claid
{
    class AccelerometerCollector : public claid::Module
    {

        private:
            Channel<AccelerationData> accelerometerDataChannel;
            CMMotionManager* motionManager;

            AccelerationData accelerationData;

            bool samplingIsRunning;

            PowerProfile currentPowerProfile;
        

            void gatherAccelerometerSample();
            void postIfEnoughData();
        
            void startSampling();
            void stopSampling();
            void restartSampling();

            void onPause() override final;
            void onResume() override final;
            void onPowerProfileChanged(PowerProfile profile) override final;

        public:

            void initialize(Properties properties);
            


       
        
    };
}