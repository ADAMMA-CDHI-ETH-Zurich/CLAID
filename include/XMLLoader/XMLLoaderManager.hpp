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

			void executeAppropriateLoaders(std::shared_ptr<XMLNode> xmlNode)
			{
				//std::cout << "EXECUTE LOADER " << loaders.size() << "\n";
				for (auto it : xmlLoaders)
				{
					XMLLoaderBase* loader = it.second;
					std::vector<std::shared_ptr<XMLNode>> desiredNodes;

					for (std::shared_ptr<XMLNode> child : xmlNode->children)
					{
						if (child->name == loader->getDesiredTag())
						{
							desiredNodes.push_back(child);
						}
						else
						{
							if(!this->doesLoaderExistForTag(child->name))
							{
								CLAID_THROW(claid::Exception, "Error! No loader is able to handle tag \"" << child->name << "\" in XML file.");
							}
						}
					} 


					loader->execute(desiredNodes);
				}
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
	static volatile RegisterHelper<className> registerHelper;

#define REGISTER_XML_LOADER(className) \
	volatile claid::XMLLoader::RegisterHelper<className> className::registerHelper(#className);\


