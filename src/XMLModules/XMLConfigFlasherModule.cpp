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
        Logger::logInfo("Unique identifier name %s", this->uniqueIdentifierChannelName.c_str());
        this->uniqueIdentifierChannel = subscribe<XMLFlasherUniqueIdentifier>(this->uniqueIdentifierChannelName, &XMLConfigFlasherModule::onUniqueIdentifierReceived, this);
        this->errorChannel = this->publish<std::string>(this->errorChannelName);
        this->flashedModulesChannel = this->publish<XMLConfigFlashedModulesDescription>(this->flashedModulesChannelName);
    }

    void XMLConfigFlasherModule::onXMLDataReceived(ChannelData<XMLConfigFlashDescription> data)
    {
        Logger::logInfo("onXMLData received 1");
        if(!this->uniqueIdentifierSet)
        {
            return;
        }
        Logger::logInfo("onXMLData received 2");

        if(data->value().uniqueIdentifier != this->uniqueIdentifier.uniqueIdentifier)
        {
            return;
        }

            Logger::logInfo("onXMLData received 3");

        try
        {
                        Logger::logInfo("onXMLData received 4");

            XMLDocument document;
            if(!document.loadFromString(data->value().xmlConfigData))
            {
                Logger::logInfo("Warning in XMLConfigFlasherModule: Failed to load XML from string, data seems to be invalid:\n%s", data->value().xmlConfigData.c_str());
                return;
            }

            Logger::logInfo("onXMLData received 5");

            std::vector<Module*> modules = CLAID_RUNTIME->parseXMLAndStartModules(document.getXMLNode());
            Logger::logInfo("onXMLData received 6");

            XMLConfigFlashedModulesDescription flashedModulesDescription;
            flashedModulesDescription.uniqueIdentifier = this->uniqueIdentifier.uniqueIdentifier;
            for(Module* module : modules)
            {
                flashedModulesDescription.flashedModuleNames.push_back(module->getModuleName());
            }
                        Logger::logInfo("onXMLData received 7");

            this->flashedModulesChannel.post(flashedModulesDescription);
        }
        catch (std::exception& e)
        {
            std::string message = e.what();
            Logger::logInfo("Error in XMLConfigFlasherModule: Parsed XML is invalid, cannot instantiate modules!\n%s", message.c_str());
            this->errorChannel.post(message);
        }
    }

    void XMLConfigFlasherModule::onUniqueIdentifierReceived(ChannelData<XMLFlasherUniqueIdentifier> data)
    {
        Logger::logInfo("Received unique identifier %u", data->value().uniqueIdentifier);
        if(this->uniqueIdentifierSet)
        {
            return;
        }
        Logger::logInfo("Registered unique identifier");

        this->uniqueIdentifier.uniqueIdentifier = data->value().uniqueIdentifier;
        this->uniqueIdentifierSet = true;
    }
}

REGISTER_MODULE(claid::XMLConfigFlasherModule)