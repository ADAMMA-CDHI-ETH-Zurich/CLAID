#include <string>
#include <unistd.h>

namespace portaible
{
    class GlobalChannelBase
    {
        public:
            virtual intptr_t getChannelDataTypeUniqueIdentifier()
            {
                return 0;
            }

            virtual std::string getChannelDataTypeUniqueIdentifierRTTIString()
            {
                return "";
            }

            GlobalChannelBase()
            {
                
            }

            virtual ~GlobalChannelBase()
            {

            }
    };
}