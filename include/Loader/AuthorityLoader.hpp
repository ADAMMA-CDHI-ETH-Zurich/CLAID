#pragma once


#include "ClassFactory/ClassFactory.hpp"
#include "RunTime/RunTime.hpp"
#include "XML/XMLDeserializer.hpp"

namespace portaible
{
	namespace Loader
	{
		class AuthorityLoader : public LoaderBase
		{
			DECLARE_LOADER(AuthorityLoader)

			public:
				AuthorityLoader() : LoaderBase("Authority")
				{

				}

				bool execute(std::vector<XMLNode*> xmlNodes) 
				{
					for (XMLNode* node : xmlNodes)
					{
						std::string className;
						if (node->getAttribute("class", className))
						{
							if (!ClassFactory::ClassFactory::getInstance()->isFactoryRegisteredForClass(className))
							{
								PORTAIBLE_THROW(portaible::Exception, "AuthorityLoader failed to load Authorities from XML. Class \"" << className << "\" was not registered and is unknown.");
							}

							Authority* authority = static_cast<Authority*>(ClassFactory::ClassFactory::getInstance()->getFactoryForClassByName(className)->getInstanceUntyped());
							XMLDeserializer deserializer(node);
							deserializer.deserializeFromNode("Authority", authority);

							std::string id;
							if (node->getAttribute("id", id))
							{
								authority->setID(id);
							}


							PORTAIBLE_RUNTIME->addAuthority(authority);
						}
					}
				}


		};
	}
}

