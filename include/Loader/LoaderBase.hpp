#pragma once

#include "XML/XMLNode.hpp"
#include <string>

namespace portaible
{
	namespace Loader
	{
		class LoaderBase
		{
		private:
			std::string desiredTag;


		public:
			LoaderBase(std::string desiredTag) : desiredTag(desiredTag)
			{

			}

			virtual bool execute(std::vector<XMLNode*> xmlNodes) = 0;

			const std::string& getDesiredTag()
			{
				return this->desiredTag;
			}
			
		};

	}
}
