#pragma once

#include "LoaderBase.hpp"

#include <string>
#include <map>


namespace portaible
{
	namespace Loader
	{
		class LoaderManager
		{

		private:
			std::map<std::string, LoaderBase*> loaders;

		public:
			template<typename T>
			void registerLoader(std::string name)
			{
				LoaderBase* base = static_cast<LoaderBase*>(new T());
				this->loaders.insert(std::make_pair(name, base));
			}

			void executeAppropriateLoaders(XMLNode* xmlNode)
			{
				//std::cout << "EXECUTE LOADER " << loaders.size() << "\n";
				for (auto it : loaders)
				{
					LoaderBase* loader = it.second;
					std::vector<XMLNode*> desiredNodes;

					for (XMLNode* child : xmlNode->children)
					{
						if (child->name == loader->getDesiredTag())
						{
							desiredNodes.push_back(child);
						}
						else
						{
							if(!this->doesLoaderExistForTag(child->name))
							{
								PORTAIBLE_THROW(portaible::Exception, "Error! No loader is able to handle tag \"" << child->name << "\" in XML file.");
							}
						}
					} 


					loader->execute(desiredNodes);
				}
			}

			bool doesLoaderExistForTag(const std::string& tag)
			{
				for(auto it : loaders)
				{
					LoaderBase* loader = it.second;
					if (tag == loader->getDesiredTag())
					{
						return true;
					}

				}
				return false;
			}

			void printLoaderTags()
			{
				for(auto it : loaders)
				{
					LoaderBase* loader = it.second;
				}
			}

		};
	}
}

#define DECLARE_LOADER(className) \
	static volatile RegisterHelper<className> registerHelper;

#define REGISTER_LOADER(className) \
	volatile portaible::Loader::RegisterHelper<className> className::registerHelper(#className);\


