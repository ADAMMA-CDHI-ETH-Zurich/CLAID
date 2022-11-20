#include "CLAID.hpp"
#include "XMLConfigFlashDescription.hpp"
namespace claid
{
    class XMLConfigFlasherModule : public Module
    {
        DECLARE_MODULE(XMLConfigFlasherModule)

        private:
            uint64_t uniqueIdentifier;
            bool uniqueIdentifierSet = false;

            std::string xmlChannelName;
            Channel<XMLConfigFlashDescription> xmlChannel;

            std::string uniqueIdentifierChannelName;
            Channel<uint64_t> uniqueIdentifierChannel;



        public:


            XMLConfigFlasherModule();
 
            void initialize();

            void onXMLDataReceived(ChannelData<XMLConfigFlashDescription> data);
            void onUniqueIdentifierReceived(ChannelData<uint64_t> uniqueIdentifier);


            Reflect(XMLConfigFlasherModule, 
                reflectMember(xmlChannelName);
                reflectMember(uniqueIdentifierChannelName);
            )
    };
}