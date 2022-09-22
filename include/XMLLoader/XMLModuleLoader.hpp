#pragma once


#include "ClassFactory/ClassFactory.hpp"
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
							if (!ClassFactory::ClassFactory::getInstance()->isFactoryRegisteredForClass(className))
							{
								PORTAIBLE_THROW(portaible::Exception, "ModuleLoader failed to load Modules from XML. Class \"" << className << "\" was not registered and is unknown.");
							}

							Module* module = static_cast<Module*>(ClassFactory::ClassFactory::getInstance()->getFactoryForClassByName(className)->getInstanceUntyped());

							if(module == nullptr)
							{
								// The class is not a module.
								PORTAIBLE_THROW(portaible::Exception, "ModuleLoader failed to load Module from XML. Class \"" << className << "\" is not a Module. Did you forget inheriting from BaseModule?");
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

