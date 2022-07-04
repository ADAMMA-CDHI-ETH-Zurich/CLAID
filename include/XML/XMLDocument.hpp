#pragma once

#include "XML/XMLParser.hpp"
#include "XMLNode.hpp"
#include "File/File.hpp"

namespace portaible
{
	class XMLDocument
	{
		private:
			XMLNode* xmlNode;

		public:
			bool loadFromFile(const File& file);

			bool loadFromFile(const std::string& file);
			bool saveToFile(const std::string& file);
			void setXMLNode(XMLNode* xmlNode);

			XMLNode* getXMLNode();

	};
}