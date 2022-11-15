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

			virtual claid::Module* instantiateModuleFromNode(std::shared_ptr<XMLNode> xmlNodes) = 0;

			const std::string& getDesiredTag()
			{
				return this->desiredTag;
			}
			
		};

	}
}
