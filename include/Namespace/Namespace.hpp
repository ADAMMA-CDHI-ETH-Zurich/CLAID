#pragma once

#include <string>

namespace claid
{
    struct Namespace
    {
        private: 
            std::string namespaceName;

        public:
            Namespace()
            {

            }

            Namespace(const std::string& namespaceName) : namespaceName(namespaceName)
            {

            }

            void prependNamespaceToString(std::string& string) const
            {
                string = this->namespaceName + std::string("/") + string;
            }


    };
}