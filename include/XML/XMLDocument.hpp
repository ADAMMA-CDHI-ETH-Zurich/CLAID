#pragma once

#include "XML/XMLParser.hpp"
#include "XMLNode.hpp"
#include "File/File.hpp"
#include "XML/XMLSerializer.hpp"
namespace claid
{
	class XMLDocument
	{
		private:
			std::shared_ptr<XMLNode> xmlNode;

		public:

			XMLDocument();
			XMLDocument(std::shared_ptr<XMLNode> node);

			bool loadFromFile(const File& file);
			bool loadFromFile(const std::string& file);
			bool saveToFile(const std::string& file);

			// Can be used to load an XML from a raw string.
			bool loadFromString(const std::string& string);

			void setXMLNode(std::shared_ptr<XMLNode> xmlNode);

			std::shared_ptr<XMLNode> getXMLNode();

			template<typename T>
			bool serializeObjectToFile(const std::string& path, T& object)
			{
				XMLSerializer serializer;
				serializer.serialize(object);
				this->xmlNode = serializer.getXMLNode();
				return this->saveToFile(path);
			}

	};
}