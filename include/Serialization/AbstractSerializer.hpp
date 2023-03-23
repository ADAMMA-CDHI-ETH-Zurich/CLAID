#pragma once
#include <string>

namespace claid
{
    class AbstractSerializer
    {
        public:
            virtual std::string getReflectorName() = 0;
            virtual ~AbstractSerializer()
            {

            }

            virtual bool getByteRepresentationOfSerializedData(std::vector<char>& data) = 0;
    };
}