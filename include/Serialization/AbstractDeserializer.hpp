#pragma once
#include <string>
namespace claid
{
    class AbstractDeserializer
    {
        public:
            virtual std::string getReflectorName() = 0;
            virtual ~AbstractDeserializer()
            {

            }

            virtual bool setByteRepresentationOfSerializedData(std::vector<char>& data) = 0;

    };
}