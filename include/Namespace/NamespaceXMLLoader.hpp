#pragma once


#include "RunTime/RunTime.hpp"
#include "XMLLoader/XMLLoaderBase.hpp"

namespace claid
{
    class NamespaceXMLLoader : public XMLLoader::XMLLoaderBase
    {
        DECLARE_XML_LOADER(NamespaceXMLLoader)

        private:
            void prependNamespaceToModule(claid::Module* module, const std::string& namespaceName)
            {
                module->prependNamespace(namespaceName);
            }

            void prependNamespaceToLoadedModules(std::vector<claid::Module*> modules, const std::string& namespaceName)
            {
                for(claid::Module* module : modules)
                {
                    module->prependNamespace(namespaceName);
                }
            }

            void checkModulesOfNamespace(std::vector<claid::Module*>& vector, const std::string& namespaceName)
			{
				for(claid::Module* entry : vector)
				{
					// Even though it should have been checked
					// by the corresponding XMLLoader already in XMLLoaderBase::instantiateModulesFromNodes),
					// we check again whether each module is valid,
					// because custom loaders can also override this function and maybe did not check carefully.
					if(entry == nullptr)
					{
						CLAID_THROW(Exception, "Error while parsing Modules from XML.\n"
						<< "At least one Module in namespace \"" << namespaceName << "\" could not be loaded and is null, which is not allowed\n.");
					}
				}
			}

			template<typename T>
			void appendToVector(std::vector<T>& vector, const std::vector<T>& toAppend)
			{
				for(const T& entry : toAppend)
				{
					vector.push_back(entry);
				}
			}

        public:
            NamespaceXMLLoader() : XMLLoaderBase("Namespace")
            {

            }

            // This is only provided because it is required that every XMLLoader implements
            // this function. The actually relevant function for the NamespaceXMLLoader, however,
            // is the instantiateModulesFromNodes function.
            // The loader returns a list of Modules contained in the namspace.
            // Therefore, this function here, that only returns one Module, only makes sense if 
            // only one Module is placed inside the namespace.
            // It probably would not be used, but as mentioned, we have to implement it.
            claid::Module* instantiateModuleFromNode(std::shared_ptr<XMLNode> node)
            {
                std::string namespaceName;
                if (node->getAttribute("name", namespaceName))
                {
                    if(node->children.size() > 1)
                    {
                        CLAID_THROW(Exception, "NamespaceXMLLoader cannot load modules in namespace \"" << namespaceName << "\".\n"
                        << "NamespaceXMLLoader::instantiateModuleFromNode was invoked manually. This function is supposed a single Module, \n"
                        << "but multiple Modules have been placed inside the namespace. Using NamespaceXMLLoader::instantiateModuleFromNode manually, \n"
                        << "only works if there is only one Module in the namespace. Otherwise, please use instantiateModulesFromNodes.");
                    }
                    else if(node->children.size() == 0)
                    {
                        CLAID_THROW(Exception, "NamespaceXMLLoader cannot load modules in namespace \"" << namespaceName << "\".\n"
                        "No modules are placed within the namespace, which is not supported. Cannot load a namespace without any modules.");
                    }

                    std::vector<Module*> loadedModules = instantiateModulesFromNamespaceNode(node);

                    if(loadedModules.size() != 1)
                    {
                        CLAID_THROW(Exception, "Error in NamespaceXMLLoader. Recursively invoked loaders to load Modules within the namespace \"" << namespaceName << "\" \n"
                        << "and expected exactly one Module to be loaded, but " << loadedModules.size() << " Modules were loaded instead.\n"
                        << "instantiateModuleFromNode only works if there is only one Module in the namespace. Otherwise, use instantiateModulesFromNodes or instantiateModulesFromNamespaceNode.");
                    }

                    claid::Module* instantiatedModule = loadedModules[0];

                    return instantiatedModule;
                }
                else
                {
                    CLAID_THROW(claid::Exception, "Error, failed to setup namespace. The \"name\" attribute is missing for the <Namespace> tag."
                    "Make sure to specify Namespaces as follows:\n"
                    "<Namespace name=\"namespace name\">\n"
                    "\t...\n"
                    "</Namespace>");
                }
            }



            // Note, that in contrast to the XMLModuleLoader, we override both, instantiateModuleFromNode and
            // instantaiteModulesFromNodes function. The NamespaceXMLLoader should return a list of Modules that are
            // contained within a namspace and not just a single module. Returning a single module only makes sense
            // if there is only one child tag in the XMLNode of the namespace.
			std::vector<claid::Module*> instantiateModulesFromNodes(std::vector<std::shared_ptr<XMLNode>>& nodes)
            {		
                std::vector<claid::Module*> allInstantiatedModules;
                for(std::shared_ptr<XMLNode> namespaceNode : nodes)
                {
                    std::vector<claid::Module*> modulesInNamespace = instantiateModulesFromNamespaceNode(namespaceNode);
                    appendToVector(allInstantiatedModules, modulesInNamespace);
                }
                
                return allInstantiatedModules;
            }

            std::vector<claid::Module*> instantiateModulesFromNamespaceNode(std::shared_ptr<XMLNode> namespaceNode)
            {
                std::string namespaceName;
                if (!namespaceNode->getAttribute("name", namespaceName))
                {
                    CLAID_THROW(claid::Exception, "Error, failed to setup namespace. The \"name\" attribute is missing for the <Namespace> tag."
                    "Make sure to specify Namespaces as follows:\n"
                    "<Namespace name=\"namespace name\">\n"
                    "\t...\n"
                    "</Namespace>");
                    
                }

                if(namespaceNode->children.size() == 0)
                {
                    CLAID_THROW(Exception, "NamespaceXMLLoader cannot load modules in namespace \"" << namespaceName << "\".\n"
                    "No modules are placed within the namespace, which is not supported. Cannot load a namespace without any modules.");
                }

                // For each of the children of the namesapceNode, the function executes
                // an appropriate loader, if there is one available that can handle the tag (if not, exception is thrown).
                std::vector<Module*> loadedModules = CLAID_RUNTIME->instantiateModulesFromXMLNode(namespaceNode);

                if(loadedModules.size() == 0)
                {
                    CLAID_THROW(Exception, "Error in NamespaceXMLLoader. Recursively invoked loaders to load Modules within the namespace \"" << namespaceName << "\" \n"
                    << "and expected at least one Module to be loaded, but " << loadedModules.size() << " Modules were loaded instead.");
                }

                checkModulesOfNamespace(loadedModules, namespaceName);
                prependNamespaceToLoadedModules(loadedModules, namespaceName);

                return loadedModules;
            }


    };
}

