#pragma once

#include "XMLLoaderBase.hpp"

#include <string>
#include <map>


namespace claid
{
	namespace XMLLoader
	{
		class XMLLoaderManager
		{

		private:
			std::map<std::string, XMLLoaderBase*> xmlLoaders;

		public:
			template<typename T>
			void registerLoader(std::string name)
			{
				XMLLoaderBase* base = static_cast<XMLLoaderBase*>(new T());
				this->xmlLoaders.insert(std::make_pair(name, base));
			}

			std::vector<claid::Module*> executeAppropriateLoaders(std::shared_ptr<XMLNode> xmlNode)
			{
				//std::cout << "EXECUTE LOADER " << loaders.size() << "\n";
				std::vector<claid::Module*> loadedModules;
				for (auto it : xmlLoaders)
				{
					XMLLoaderBase* loader = it.second;
					std::vector<std::shared_ptr<XMLNode>> desiredNodes;

					for (std::shared_ptr<XMLNode> child : xmlNode->children)
					{
						if (child->name == loader->getDesiredTag())
						{
							std::cout << "Instantiate 1\n" << std::flush;
							claid::Module* loadedModule = loader->instantiateModuleFromNode(child);
							std::cout << "Instantiate 2\n" << std::flush;

							if(loadedModule == nullptr)
							{
								CLAID_THROW(claid::Exception, "Error in loading Module from XML. Cannot load Module from XML Node " << child->name);
							}
							loadedModules.push_back(loadedModule);			
							std::cout << "Instantiate 3\n" << std::flush;
				
						}
						else
						{
							if(!this->doesLoaderExistForTag(child->name))
							{
								CLAID_THROW(claid::Exception, "Error! No loader is able to handle tag \"" << child->name << "\" in XML file.");
							}
						}
					} 
				}
				std::cout << "Instantiate 4\n" << std::flush;

				return loadedModules;
				
			}

			bool doesLoaderExistForTag(const std::string& tag)
			{
				for(auto it : xmlLoaders)
				{
					XMLLoaderBase* loader = it.second;
					if (tag == loader->getDesiredTag())
					{
						return true;
					}

				}
				return false;
			}

			void printLoaderTags()
			{
				for(auto it : xmlLoaders)
				{
					XMLLoaderBase* loader = it.second;
				}
			}

		};
	}
}

#define DECLARE_XML_LOADER(className) \
	static volatile claid::XMLLoader::RegisterHelper<className> registerHelper;

#define REGISTER_XML_LOADER(className) \
	volatile claid::XMLLoader::RegisterHelper<className> className::registerHelper(#className);\


