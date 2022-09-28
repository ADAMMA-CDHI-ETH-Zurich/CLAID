#pragma once


#include "RunTime/RunTime.hpp"
#include "XML/XMLDeserializer.hpp"

namespace portaible
{
	namespace XMLLoader
	{
		class XMLModuleLoader : public XMLLoaderBase
		{
			DECLARE_XML_LOADER(XMLModuleLoader)

			public:
				XMLModuleLoader() : XMLLoaderBase("Module")
				{

				}

				bool execute(std::vector<std::shared_ptr<XMLNode>> xmlNodes) 
				{
					for (std::shared_ptr<XMLNode> node : xmlNodes)
					{
						std::string className;
						if (node->getAttribute("class", className))
						{
							if (!ModuleFactory::ModuleFactory::getInstance()->isFactoryRegisteredForModule(className))
							{
								PORTAIBLE_THROW(portaible::Exception, "ModuleLoader failed to load Module from XML. Class \"" << className << "\" was not registered and is unknown.");
							}

							Module* module = dynamic_cast<Module*>(ModuleFactory::ModuleFactory::getInstance()->getFactoryForModuleByName(className)->getInstanceUntyped());

							if(module == nullptr)
							{
								// The class is not a Module.
								PORTAIBLE_THROW(portaible::Exception, "ModuleLoader failed to load Module from XML. Class \"" << className << "\" is not a Module. Did you forget inheriting from portaible::Module?");
							}

							XMLDeserializer deserializer(node);
							deserializer.deserializeFromNode("Module", module);

							std::string id;
							if (node->getAttribute("id", id))
							{
								module->setID(id);
							}

							PORTAIBLE_RUNTIME->addModule(module);
						}
					}
					return true;
				}


		};
	}
}

