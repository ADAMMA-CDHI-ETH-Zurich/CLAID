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
            virtual void writeHeaderToFile(const std::string& path, bool append = true)
            {
            }

            virtual void writeDataToFile(const std::string& path, bool append = true)
            {
                CLAID_THROW(Exception, "Serializer " << this->getReflectorName() << "was used to serialize some data.\n"
                << "It was requested to store the serialized data in file \"" << path << "\", however serializer "
                << this->getReflectorName() << " does not support writing serialized data to a file.");
            }

            virtual void forceReset()
            {

            }
            
    };
}