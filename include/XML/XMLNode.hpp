#pragma once
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <algorithm>
#include "Logger/Logger.hpp"
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
            
            intptr_t hierarchy = 0;

            std::vector<std::shared_ptr<XMLNode>> children;
            
            std::map<std::string, std::string> attributes;

            XMLNode(std::shared_ptr<XMLNode> parent, const std::string& name) : name(name)
            {
                if(parent != nullptr)
                {
                    this->hierarchy = parent->hierarchy + 1;
                }
            }

            virtual ~XMLNode()
            {

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

            virtual void contentAsString(std::string& string)
            {
                string = "";
                for(std::shared_ptr<XMLNode> child : children)
                {
                    std::string tmp;
                    child->contentAsString(tmp);
                    string += tmp + "\n";
                }
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


        
                        static std::shared_ptr<XMLNode> merge(std::vector<std::shared_ptr<XMLNode>>& nodes)
            {
                if(nodes.size() == 0)
                {
                    return nullptr;
                }

                if(nodes.size() == 1)
                {
                    return nodes[0];
                }

                std::shared_ptr<XMLNode> rootNode = nodes[0];

                for(size_t i = 1; i < nodes.size(); i++)
                {
                    std::shared_ptr<XMLNode> currentNode = nodes[i];

                    for(std::shared_ptr<XMLNode> child : currentNode->children)
                    {
                        rootNode->addChild(child);
                    }
                }

                return rootNode;
            }

            // If collectionItemTag is "item", for example, all nodes would be added like follows:
            // <item>node1</item>
            // <item>node2</item> etc.
            static std::shared_ptr<XMLNode> mergeAsCollection(std::vector<std::shared_ptr<XMLNode>>& nodes, std::string collectionItemTag)
            {
                std::shared_ptr<XMLNode> rootNode = std::make_shared<XMLNode>(nullptr, "root");
                for(std::shared_ptr<XMLNode> currentNode : nodes)
                {
                    std::shared_ptr<XMLNode> itemNode = std::make_shared<XMLNode>(rootNode, collectionItemTag);
                    
                    for(std::shared_ptr<XMLNode> child : currentNode->children)
                    {
                        itemNode->addChild(child);
                    }
                    rootNode->addChild(itemNode);
                }

                return rootNode;
            }
    };
}