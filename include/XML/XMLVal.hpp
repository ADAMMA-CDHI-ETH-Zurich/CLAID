#pragma once
#include "XMLNode.hpp"
#include <string>
#include <memory.h>

namespace claid
{
    class XMLVal : public XMLNode
    {
        protected:
                std::string value;

        public:



            XMLVal(std::shared_ptr<XMLNode> parent, const std::string& name, const std::string& value) : XMLNode(parent, name), value(value)
            {
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