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

			void checkModulesReturnedByLoader(std::vector<claid::Module*>& vector, const std::string& xmlTag)
			{
				for(claid::Module* entry : vector)
				{
					// Even though it should have been checked
					// by the corresponding XMLLoader already in XMLLoaderBase::instantiateModulesFromNodes),
					// we check again whether each module is valid,
					// because custom loaders can also override this function and maybe did not check carefully.
					if(entry == nullptr)
					{
						CLAID_THROW(Exception, "Error while parsing Modules from XML.\n"
						<< "Loader for tag \"" << xmlTag << "\" returned a Module which is null.\n"
						<< "This is not allowed.");
					}
				}
			}

			template<typename T>
			void appendToVector(std::vector<T>& vector, const std::vector<T>& toAppend)
			{
				for(const T& entry : toAppend)
				{
					vector.push_back(entry);
				}
			}

		public:
			template<typename T>
			void registerLoader(std::string name)
			{
				XMLLoaderBase* base = static_cast<XMLLoaderBase*>(new T());
				this->xmlLoaders.insert(std::make_pair(name, base));
			}

			std::vector<claid::Module*> executeAppropriateLoaders(std::shared_ptr<XMLNode> xmlNode)
			{
				if(xmlNode.get() == nullptr)
				{
					CLAID_THROW(claid::Exception, "Error, cannot invoke XMLLoaders to load Modules, the provided XMLNode is empty.");
				}
				//std::cout << "EXECUTE LOADER " << loaders.size() << "\n";
				std::vector<claid::Module*> loadedModules;

				std::map<std::string, std::vector<std::shared_ptr<XMLNode>>> nodesForEachTag;

				for (std::shared_ptr<XMLNode> child : xmlNode->children)
				{
					const std::string& xmlTag = child->name;
					nodesForEachTag[xmlTag].push_back(child);
				}
				
				for(std::pair<std::string, std::vector<std::shared_ptr<XMLNode>>> entry : nodesForEachTag)
				{
					const std::string& xmlTag = entry.first;
					if(!this->doesLoaderExistForTag(xmlTag))
					{
						CLAID_THROW(claid::Exception, "Error while loading from XML. Tag \"" << xmlTag << "\" in XML file is unknown.\n"
						<< "No loader is able to handle this tag.");
					}

					XMLLoaderBase* loader = getLoaderForTag(xmlTag);

					std::vector<claid::Module*> modules = loader->instantiateModulesFromNodes(entry.second);
					checkModulesReturnedByLoader(modules, xmlTag);
					appendToVector(loadedModules, modules);
				}

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

			XMLLoaderBase* getLoaderForTag(const std::string& tag)
			{
				for(auto it : xmlLoaders)
				{
					XMLLoaderBase* loader = it.second;
					if (tag == loader->getDesiredTag())
					{
						return loader;
					}

				}

				CLAID_THROW(Exception, "Error, cannot find XMLLoader for tag \"" << tag << "\".\n"
					<< "No loader can handle this tag.");
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


