#pragma once
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <algorithm>

namespace claid
{
    class XMLNode 
    {
        protected: 
            std::string getTabs()
            {
                std::string tabs = "";
                for(size_t i = 0; i < this->hierarchy; i++)
                {
                    tabs += "\t";
                }
                return tabs;
            }

        public:
            std::string name;
            std::shared_ptr<XMLNode> parent;
            
            intptr_t hierarchy = 0;

            std::vector<std::shared_ptr<XMLNode>> children;
            
            std::map<std::string, std::string> attributes;

            XMLNode(std::shared_ptr<XMLNode> parent, const std::string& name) : name(name), parent(parent)
            {
                if(parent != nullptr)
                {
                    this->hierarchy = parent->hierarchy + 1;
                }
            }

            std::shared_ptr<XMLNode> findChild(const std::string& name)
            {
                for(std::shared_ptr<XMLNode> child : children)
                {
                    if(child->name == name)
                    {
                        return child;
                    }
                }
                return nullptr;
            }

            // This will get overriden by specific XMLNodes, like XMLVal, which are leaves of the tree.
            // For all non-leaf nodes, the corresponding functions of the children nodes need to be called.
            virtual void toString(std::string& string)
            {
                std::string openingTag;
                std::string closingTag;

                this->getOpeningTag(openingTag);
                this->getClosingTag(closingTag);


                string += this->getTabs() + openingTag + "\n";
                for(std::shared_ptr<XMLNode> child : children)
                {
                    std::string tmp;
                    child->toString(tmp);
                    string += tmp + "\n";
                }
                string += this->getTabs() + closingTag;
            }

            

            // Needs to be implemented by children! Will be used by XMLDeserializer.
            virtual bool fromString(const std::string& string) 
            {
                return false;
            }

            void addChild(std::shared_ptr<XMLNode> child)
            {
                this->children.push_back(child);
            }


            virtual void getOpeningTag(std::string& tag)
            {
                // Add attributes
                tag = std::string("<") + this->name;
                
                for(const std::pair<std::string, std::string>& attribute : attributes)
                {
                    tag = tag + std::string(" ") + attribute.first + std::string("=\"") + attribute.second + std::string("\"");
                }
                tag = tag + std::string(">");
            }

            virtual void getClosingTag(std::string& tag)
            {
                tag = std::string("</") + this->name + std::string(">");
            }

            bool hasAttribute(const std::string& name)
            {
                return this->attributes.find(name) != this->attributes.end();
            }

            bool getAttribute(const std::string& name, std::string& attribute)
            {
                auto it = this->attributes.find(name);

                if (it == this->attributes.end())
                {
                    return false;
                }

                attribute = it->second;

                return true;
            }

            void setAttribute(const std::string& name, const std::string& attribute)
            {
                this->attributes.insert(std::make_pair(name, attribute));
            }
    };
}