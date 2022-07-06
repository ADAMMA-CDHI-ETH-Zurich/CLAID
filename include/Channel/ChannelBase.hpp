#include <string>
#include <unistd.h>

namespace portaible
{
    class ChannelBase
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

            ChannelBase()
            {
                
            }

            virtual ~ChannelBase()
            {

            }
    };
}