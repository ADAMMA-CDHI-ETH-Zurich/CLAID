#pragma once

#include "XML/XMLNode.hpp"
#include <string>

namespace claid
{
	namespace XMLLoader
	{
		class XMLLoaderBase
		{
		private:
			std::string desiredTag;


		public:
			XMLLoaderBase(std::string desiredTag) : desiredTag(desiredTag)
			{

			}

			
			// This function *has* to be overriden by every loader.
			// It implements the loading routine for the corresponding tag.
			virtual claid::Module* instantiateModuleFromNode(std::shared_ptr<XMLNode> xmlNodes) = 0;
			
			// This function *can* be overriden by every loader.
			// If overriden, it allows to handle multiple modules within one tag.
			// This is useful for the NamespaceXMLLoader, for example, as it needs to recursively call
			// the corresponding loader for each node within the namespace.
			virtual std::vector<claid::Module*> instantiateModulesFromNodes(std::vector<std::shared_ptr<XMLNode>>& nodes)
			{
				std::vector<claid::Module*> instantiatedModules;
				for(std::shared_ptr<XMLNode> node : nodes)
				{
					claid::Module* parsedModule = this->instantiateModuleFromNode(node);

					if(parsedModule == nullptr)
					{
						CLAID_THROW(claid::Exception, "Error in XMLLoader for tag \"" << desiredTag << "\": The loaded Module is null.\n"
						<< "Hence, the associated loader returned an invalid Module, which is not allowed.");
					}
					instantiatedModules.push_back(parsedModule);
				}

				return instantiatedModules;
			}

			const std::string& getDesiredTag()
			{
				return this->desiredTag;
			}
			
		};

	}
}
