#include "CLAID.hpp"
#include "XMLConfigFlashDescription.hpp"
#include "XMLConfigFlashedModulesDescription.hpp"
#include "XMLFlasherUniqueIdentifier.hpp"
namespace claid
{
    class XMLConfigFlasherModule : public Module
    {

        private:
            XMLFlasherUniqueIdentifier uniqueIdentifier;
            bool uniqueIdentifierSet = false;

            std::string xmlChannelName;
            Channel<XMLConfigFlashDescription> xmlChannel;

            std::string uniqueIdentifierChannelName;
            Channel<XMLFlasherUniqueIdentifier> uniqueIdentifierChannel;

            std::string errorChannelName;
            Channel<std::string> errorChannel;

            std::string flashedModulesChannelName;
            Channel<XMLConfigFlashedModulesDescription> flashedModulesChannel;


        public:

            XMLConfigFlasherModule();
 
            void initialize();

            void onXMLDataReceived(ChannelData<XMLConfigFlashDescription> data);
            void onUniqueIdentifierReceived(ChannelData<XMLFlasherUniqueIdentifier> uniqueIdentifier);


            Reflect(XMLConfigFlasherModule, 
                reflectMember(xmlChannelName);
                reflectMember(uniqueIdentifierChannelName);
                reflectMember(errorChannelName);
                reflectMember(flashedModulesChannelName);
            )
    };
}