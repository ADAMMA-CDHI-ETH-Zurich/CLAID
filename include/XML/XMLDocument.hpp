#pragma once

#include "XML/XMLParser.hpp"
#include "XMLNode.hpp"
#include "File/File.hpp"

namespace claid
{
	class XMLDocument
	{
		private:
			std::shared_ptr<XMLNode> xmlNode;

		public:
			bool loadFromFile(const File& file);

			bool loadFromFile(const std::string& file);
			bool saveToFile(const std::string& file);
			void setXMLNode(std::shared_ptr<XMLNode> xmlNode);

			std::shared_ptr<XMLNode> getXMLNode();

	};
}