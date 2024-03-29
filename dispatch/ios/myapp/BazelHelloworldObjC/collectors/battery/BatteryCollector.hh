#include "dispatch/core/CLAID.hh"
#include "dispatch/proto/sensor_data_types.pb.h"

// IMPORTAT: Test this only on real device, emulator produces wrong values

namespace claid
{
    class BatteryCollector : public claid::Module
    {
        
        private:
            Channel<BatteryData> batteryDataChannel;

            uint16_t samplingPeriod;

            BatteryData batteryData;
            void initialize(Properties properties);
            void sampleBatteryData();
            void postIfEnoughData();

        public:

            static void annotateModule(ModuleAnnotator& annotator);
       
          
    };
}
