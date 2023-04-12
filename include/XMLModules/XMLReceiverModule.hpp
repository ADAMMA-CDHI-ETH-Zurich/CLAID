#include "CLAID.hpp"

namespace claid
{
    class XMLReceiverModule : public Module
    {

        private:
            std::string xmlChannelName;
            Channel<std::string> xmlChannel;

            // Specifies whether an exception shall be thrown if XML is invalid.
            bool throwExceptionOnFailure = false;

        public:


            XMLReceiverModule();
            XMLReceiverModule(std::string xmlChannelName, bool throwExceptionOnFailure);

            void initialize();

            void onXMLDataReceived(ChannelData<std::string> data);


            Reflect(XMLReceiverModule, 
                reflectMember(xmlChannelName);
                reflectMemberWithDefaultValue(throwExceptionOnFailure, false);
            )
    };
}