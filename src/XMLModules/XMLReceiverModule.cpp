#include "include/XMLModules/XMLReceiverModule.hpp"
#include "XML/XMLDocument.hpp"

namespace claid
{
    XMLReceiverModule::XMLReceiverModule()
    {

    }

    XMLReceiverModule::XMLReceiverModule(std::string xmlChannelName, bool throwExceptionOnFailure) : xmlChannelName(xmlChannelName), throwExceptionOnFailure(throwExceptionOnFailure)
    {

    }

    void XMLReceiverModule::initialize()
    {
        this->xmlChannel = subscribe<std::string>(this->xmlChannelName, &XMLReceiverModule::onXMLDataReceived, this);
    }

    void XMLReceiverModule::onXMLDataReceived(ChannelData<std::string> data)
    {
        XMLDocument document;
        if(!document.loadFromString(data->value()))
        {
            if(this->throwExceptionOnFailure)
            {   
                CLAID_THROW(claid::Exception, "Error in XMLReceiverModule. Cannot load XML from string, data seems to be invalid:\n " << data->value());
            }
            else
            {
                Logger::printfln("Warning in XMLReceiverModule: Failed to load XML from string, data seems to be invalid:\n%s", data->value().c_str());
            }
        }
    }
}

REGISTER_MODULE(claid::XMLReceiverModule)