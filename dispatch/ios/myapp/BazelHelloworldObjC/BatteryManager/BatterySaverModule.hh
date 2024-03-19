#pragma once

#include "dispatch/core/Module/ManagerModule.hh"
#include "dispatch/proto/claidservice.grpc.pb.h"

#import "Foundation/Foundation.h"
#import <UIKit/UIKit.h>

using claidservice::PowerSavingStrategy;
using claidservice::PowerProfile;

using namespace claid;

namespace claid
{
    class BatterySaverModule : public ManagerModule
    {
        private:
            std::vector<PowerSavingStrategy> powerSavingStrategies;


            void onBatteryLevelChanged(float level);
            void getCurrentBatteryLevel();
            void executeStrategy(int index);

            float lastBatteryLevel = -1.0;

            int currentlyActiveStrategy = -1;



        public:
            void initialize(const std::map<std::string, std::string>& properties);

            BatterySaverModule();
            ~BatterySaverModule();
    };  
}