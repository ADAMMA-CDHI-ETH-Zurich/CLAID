#include "XMLModules/XMLConfigFlasherModule.hpp"
#include "XML/XMLDocument.hpp"

namespace claid
{
    XMLConfigFlasherModule::XMLConfigFlasherModule()
    {

    }

    void XMLConfigFlasherModule::initialize()
    {
        this->xmlChannel = subscribe<XMLConfigFlashDescription>(this->xmlChannelName, &XMLConfigFlasherModule::onXMLDataReceived, this);
        this->uniqueIdentifierChannel = subscribe<uint64_t>(this->uniqueIdentifierChannelName, &XMLConfigFlasherModule::onUniqueIdentifierReceived, this);
    }

    void XMLConfigFlasherModule::onXMLDataReceived(ChannelData<XMLConfigFlashDescription> data)
    {
        if(!this->uniqueIdentifierSet)
        {
            return;
        }

        if(data->value().uniqueIdentifier != this->uniqueIdentifier)
        {
            return ;
        }

    
        try
        {
            XMLDocument document;
            if(!document.loadFromString(data->value().xmlConfigData))
            {
                Logger::printfln("Warning in XMLConfigFlasherModule: Failed to load XML from string, data seems to be invalid:\n%s", data->value().xmlConfigData.c_str());
                return;
            }


            CLAID_RUNTIME->parseXMLAndStartModules(document.getXMLNode());
        }
        catch (std::exception& e)
        {
            std::string message = e.what();
            Logger::printfln("Error in XMLConfigFlasherModule: Parsed XML is invalid, cannot instantiate modules!\n%s", message.c_str());
        }
    }

    void XMLConfigFlasherModule::onUniqueIdentifierReceived(ChannelData<uint64_t> data)
    {
        if(this->uniqueIdentifierSet)
        {
            return;
        }

        this->uniqueIdentifier = data->value();
        this->uniqueIdentifierSet = true;
    }
}

REGISTER_MODULE(claid::XMLConfigFlasherModule)