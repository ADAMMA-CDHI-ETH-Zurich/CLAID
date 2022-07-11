#include "XML/XMLDocument.hpp"
#include <fstream>

namespace portaible
{
	bool XMLDocument::loadFromFile(const File& file)
	{
		XMLParser xmlParser;

		return (!xmlParser.parseFromString(file.getContent(), this->xmlNode));
	}

	bool XMLDocument::loadFromFile(const std::string& filePath)
	{
		std::ifstream file(filePath);
		std::string fileContent;

		if (!file.is_open())
		{
			printf("Error! Could not open file \"%s\".", filePath.c_str());
			return false;
		}

		file.seekg(0, std::ios::end);
		fileContent.reserve(file.tellg());
		file.seekg(0, std::ios::beg);

		fileContent.assign((std::istreambuf_iterator<char>(file)),
			std::istreambuf_iterator<char>());

		XMLParser xmlParser;

		return (!xmlParser.parseFromString(fileContent, this->xmlNode));
		
	}

	bool XMLDocument::saveToFile(const std::string& filePath)
	{
		std::string data;
		if (this->xmlNode == nullptr)
		{
			return false;
		}

		this->xmlNode->toString(data);

		std::ofstream file(filePath);

		if (!file.is_open())
		{
			printf("Error! Could not open file \"%s\".", filePath.c_str());
			return false;
		}

		file.write(data.c_str(), data.length());

		return true;

	}

	void XMLDocument::setXMLNode(std::shared_ptr<XMLNode> xmlNode)
	{
		this->xmlNode = xmlNode;
	}

	std::shared_ptr<XMLNode> XMLDocument::getXMLNode()
	{
		return this->xmlNode;
	}
}


