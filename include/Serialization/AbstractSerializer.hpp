#pragma once
#include <string>
#include "TypeChecking/TypeCheckingFunctions.hpp"
#include "Exception/Exception.hpp"
namespace claid
{
    class AbstractSerializer
    {
        public:
            virtual std::string getReflectorName() = 0;
            virtual ~AbstractSerializer()
            {

            }

            // Not all Serializers might require a header.
            // A header exists only once per file.
            // E.g., in a CSV, a header would be written only once at the start.
            virtual void getHeaderWriteableToFile(std::vector<char>& data)
            {
                data.clear();
            }

            virtual void getDataWriteableToFile(std::vector<char>& data)
            {
                CLAID_THROW(Exception, "Serializer " << this->getReflectorName() << "was used to serialize some data.\n"
                << "In order to write data to a file, a writeable representation was requested.\n"
                << "However, Serializer " << this->getReflectorName() << " does not support to transform serialized data to a representation that can be written to a file.");
            }

            virtual void forceReset()
            {

            }
            
    };
}