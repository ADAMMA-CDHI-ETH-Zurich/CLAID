#pragma once
#include "Binary/BinaryData.hpp"
#include "TypeChecking/TypeCheckingFunctions.hpp"
#include "Exception/Exception.hpp"
namespace claid
{
    class BinaryDataReader
    {   
        private:
            const BinaryData& data;
            const char* readingPtr = nullptr;
            const char* dataEndPtr = nullptr;
            
            bool validData = false;

            template<typename T>
            void getBinarySize(int32_t& size) const
            {
                size = sizeof(T);
            }

            template<typename T>
            const T& fromBinary(const char* ptr) const
            {
                return *reinterpret_cast<const T*>(ptr);
            }

        public:
   
            
            BinaryDataReader(const BinaryData& binaryData) : data(binaryData)
            {
                this->resetReader();
                this->validData = true;
            }

            // Should only be enabled for primitive types (and byte, which we defined ourselves).
            template <typename T>
            typename std::enable_if<std::is_arithmetic<T>::value>::type // type of enable_if is void, if value is true, if not specified otherwise
            read(T& value)
            {
                if(!this->validData)
                {
                    CLAID_THROW(Exception, "Error, cannot read from binary data using BinaryDataReader. No valid data has been set!");
                }

                int32_t size;
                getBinarySize<T>(size);

                // Test if we run out of bounds if we continue reading from the data.
                if(this->readingPtr + size > this->dataEndPtr)
                {
                    CLAID_THROW(Exception, "Error reading from BinaryData. Trying to read value of type \"" << TypeChecking::getCompilerSpecificCompileTypeNameOfClass<T>() << "\" which requires " << size << " bytes of data, " 
                    << "but only " << std::distance(this->readingPtr, this->dataEndPtr) << " bytes are left to read from the binary data (i.e. BinaryData contains less bytes than required).\n"
                    << "Total number of bytes available is: " << this->data.getNumBytes() << ", number of bytes previously read already is " << std::distance(this->data.getConstRawData(), this->readingPtr));
                }

                value = fromBinary<T>(this->readingPtr);

                this->readingPtr += size;
            }

            void readBytes(char*& ptr, size_t numBytes)
            {
                if(!this->validData)
                {
                    CLAID_THROW(Exception, "Error, cannot read from binary data using BinaryDataReader. No valid data has been set!");
                }

                if(this->readingPtr + numBytes > this->dataEndPtr)
                {
                    CLAID_THROW(Exception, "Error reading from BinaryData. Trying to read " << numBytes << " bytes of data, "  
                    << "but only " << std::distance(this->readingPtr, this->dataEndPtr) << " bytes are left to read from the binary data (i.e. BinaryData contains less bytes than required).");
                }

                for(size_t i = 0; i < numBytes; i++)
                {
                    ptr[i] = *this->readingPtr;
                    this->readingPtr++;
                }
            }

            void readString(std::string& value)
            {
                if(!this->validData)
                {
                    CLAID_THROW(Exception, "Error, cannot read from binary data using BinaryDataReader. No valid data has been set!");
                }

                // Load length of string
                int32_t size;
                this->read(size);

                value.resize(size);
                // Store characters

                for(int32_t i = 0; i < size; i++)
                {
                    this->read(value[i]);
                }                                

            }

      
            void resetReader()
            {
                this->readingPtr = this->data.getConstRawData();
                this->dataEndPtr = this->readingPtr + this->data.getNumBytes();
            }
    };  
}