#pragma once
#include "dispatch/core/CLAID.hh"
#include "dispatch/proto/sensor_data_types.pb.h"
#include "LocationTracker.hh"
#include "../../Permissions/LocationPermission.hh"

using namespace claidservice;

namespace claid
{
    class LocationCollector : public claid::Module
    {

        private:
            LocationTracker* locationTracker;
            LocationPermission* locationPermission;
            claid::Channel<LocationData> locationDataChannel;

            bool isRunning = false;

            PowerProfile currentPowerProfile;

            void sampleLocation();

            void startLocationUpdates();
            void stopLocationUpdates();
            void restartLocationUpdates();

            void onPause() override final;
            void onResume() override final;
            void onPowerProfileChanged(PowerProfile profile) override final;

        public:

            void initialize(Properties properties);
                



    };
}