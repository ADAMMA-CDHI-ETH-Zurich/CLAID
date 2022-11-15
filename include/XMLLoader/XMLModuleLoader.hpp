#pragma once


#include "RunTime/RunTime.hpp"
#include "XML/XMLDeserializer.hpp"

namespace claid
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

				claid::Module* instantiateModuleFromNode(std::shared_ptr<XMLNode> node) 
				{		
					std::string className;
					if (node->getAttribute("class", className))
					{
						if (!ModuleFactory::ModuleFactory::getInstance()->isFactoryRegisteredForModule(className))
						{
							CLAID_THROW(claid::Exception, "ModuleLoader failed to load Module from XML. Class \"" << className << "\" was not registered and is unknown.");
						}

						Module* module = dynamic_cast<Module*>(ModuleFactory::ModuleFactory::getInstance()->getFactoryForModuleByName(className)->getInstanceUntyped());

						if(module == nullptr)
						{
							// The class is not a Module.
							CLAID_THROW(claid::Exception, "ModuleLoader failed to load Module from XML. Class \"" << className << "\" is not a Module. Did you forget inheriting from claid::Module?");
						}

						XMLDeserializer deserializer(node);
						deserializer.deserializeExistingPolymorphicObject(className, module);

						std::string id;
						if (node->getAttribute("id", id))
						{
							module->setID(id);
						}

						CLAID_RUNTIME->addModule(module);
						
						return module;
					}
					else
					{
						CLAID_THROW(claid::Exception, "Error, failed to load (C++) Module when parsing XML configuration. The \"class\" attribute is missing for the <Module> tag."
						"Make sure to specify Modules as follows:\n"
						"<Module class=\"namespace.class\">\n"
						"\t...\n"
						"</Module>");
						return nullptr;
					}
					
					return nullptr;
				}


		};
	}
}

