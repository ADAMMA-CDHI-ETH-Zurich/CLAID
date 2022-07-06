#pragma once


#include "ClassFactory/ClassFactory.hpp"
#include "RunTime/RunTime.hpp"
#include "XML/XMLDeserializer.hpp"

namespace portaible
{
	namespace Loader
	{
		class ModuleLoader : public LoaderBase
		{
			DECLARE_LOADER(ModuleLoader)

			public:
				ModuleLoader() : LoaderBase("Module")
				{

				}

				bool execute(std::vector<XMLNode*> xmlNodes) 
				{
					for (XMLNode* node : xmlNodes)
					{
						std::string className;
						if (node->getAttribute("class", className))
						{
							if (!ClassFactory::ClassFactory::getInstance()->isFactoryRegisteredForClass(className))
							{
								PORTAIBLE_THROW(portaible::Exception, "ModuleLoader failed to load Modules from XML. Class \"" << className << "\" was not registered and is unknown.");
							}

							Module* module = static_cast<Module*>(ClassFactory::ClassFactory::getInstance()->getFactoryForClassByName(className)->getInstanceUntyped());
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

