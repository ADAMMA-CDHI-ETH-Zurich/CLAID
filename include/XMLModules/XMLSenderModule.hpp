#include "CLAID.hpp"

namespace claid
{
    // Posts configs to requested channels.
    // E.g., another Module can request for a config to be sent to a certain channel.
    class XMLSenderModule : public Module
    {
        DECLARE_MODULE(XMLSenderModule)

        private:
         
            std::string configPath;
            std::string requestChannelName;


            Channel<XMLConfigRequest> requestChannel;
            XMLDocument xmlDocument;


        public:


            

            void initialize();



            Reflect(XMLSenderModule,
                reflectMember(configPath);
            )
    };
}