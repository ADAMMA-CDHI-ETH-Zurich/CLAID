#pragma once
#include "XMLNode.hpp"
#include <string>

namespace portaible
{
    class XMLVal : public XMLNode
    {
        protected:
                std::string value;

        public:



            XMLVal(XMLNode* parent, const std::string& name, const std::string& value) : XMLNode(parent, name), value(value)
            {
                printf("constr %s\n", name.c_str());
            }

            virtual void toString(std::string& string)
            {
                std::string openingTag;
                std::string closingTag;
                this->getOpeningTag(openingTag);
                this->getClosingTag(closingTag);
                string = this->getTabs() + openingTag + value + closingTag;
            }


            virtual bool fromString(const std::string& string)
            {
                this->value = string;
                //parseFromString(this->value, string);
                return true;
            }

            const std::string& getValue()
            {
                return this->value;
            }

    };
}