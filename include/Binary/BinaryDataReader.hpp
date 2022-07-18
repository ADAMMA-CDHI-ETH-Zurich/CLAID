#pragma once
#include "Binary/BinaryData.hpp"

namespace portaible
{
    class BinaryDataReader
    {   
        private:
            BinaryData* data = nullptr;
            char* readingPtr = nullptr;

        public:
   
            BinaryDataReader() : data(nullptr)
            {

            }

            BinaryDataReader(BinaryData* binaryData) : data(binaryData)
            {
                this->resetReader();
            }

            // Should only be enabled for primitive types
            template <typename T>
            typename std::enable_if<std::is_arithmetic<T>::value>::type // type of enable_if is void, if value is true, if not specified otherwise
            read(T& value)
            {
                size_t size;
                value = fromBinary<T>(this->readingPtr, size);

                this->readingPtr += size;
            }

            void readString(std::string& value)
            {
                // Load length of string
                size_t size;
                this->read(size);

                value.resize(size);
                // Store characters

                for(size_t i = 0; i < size; i++)
                {
                    this->read(value[i]);
                }

                                

            }

            template<typename T>
            const T& fromBinary(char* ptr, size_t& size) const
            {
                size = sizeof(T);
                return *reinterpret_cast<T*>(ptr);
            }

            void resetReader()
            {
                this->readingPtr = this->data->getRawData();
            }
    };  
}