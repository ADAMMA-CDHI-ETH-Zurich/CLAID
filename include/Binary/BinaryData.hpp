#pragma once
#include <vector>
#include "Reflection/SplitReflectInType.hpp"
#include "Exception/Exception.hpp"
#include <iostream>
#include <fstream>


namespace claid
{
    class BinaryData
    {
        
       
        public:
            template<typename Reflector>
            void reflect(Reflector& r)
            {
                splitReflectInType(r, *this);
            }

            template<typename Reflector>
            void reflectRead(Reflector& r)
            {
                int32_t bytes;
                r.member("NumBytes", bytes, "");

                this->resize(bytes);

                char* dataPtr = this->getRawData();
                r.read(dataPtr, bytes);
            }

            template<typename Reflector>
            void reflectWrite(Reflector& r)
            {
                int32_t bytes = this->data.size();
                r.member("NumBytes", bytes, "");

                r.write(this->getRawData(), bytes);
            }

            // Should only be enabled for primitive types and byte (which we defined ourselves).
            template <typename T>
            typename std::enable_if<std::is_arithmetic<T>::value>::type // type of enable_if is void, if value is true, if not specified otherwise. If false, then type does not exist (see implementation of enable_if).
            store(const T& value)
            {
                size_t size;
                const char* binaryData = toBinary<T>(value, size);

                this->store(binaryData, size);
            }

            void store(const char* data, size_t size)
            {
                for(size_t i = 0; i < size; i++)
                {
                    this->data.push_back(data[i]);
                }
            }

            void storeString(const std::string& value)
            {
                // Store length of string
                int32_t size = value.size();
                this->store(size);

                // Store characters
                for(int32_t i = 0; i < size; i++)
                {
                    this->store(value[i]);
                }
            }

            void insertBytes(const char* bytes, size_t numBytes)
            {
                for(size_t i = 0; i < numBytes; i++)
                {
                    this->data.push_back(*bytes);
                    bytes++;
                }
            }

            void resize(size_t size)
            {
                size_t index = 0;
                this->data.resize(size);      
            }

            char* getRawData()
            {
                return this->data.data();
            }

            const char* getConstRawData() const
            {
                return this->data.data();
            }

            std::vector<char>& getData()
            {
                return this->data;
            }

            size_t getNumBytes() const
            {
                return this->data.size();
            }

            void clear()
            {
                this->data.clear();
            }

            void loadFromFile(const std::string& path)
            {
                   
                std::fstream file(path, std::ios::in | std::ios::binary);
                if(!file.is_open())
                {
                    CLAID_THROW(claid::Exception, "Error, cannot load binary data from \"" << path << "\".\n"
                    << "Could not open File for writing.");
                }

                file.seekg(0, std::ios::end);
                size_t numBytes = file.tellg();
                file.seekg(0, std::ios::beg);

                this->resize(numBytes);

                file.read(this->data.data(), numBytes);
            }

            void saveToFile(const std::string& path) const
            {
                std::fstream file(path, std::ios::out | std::ios::binary);
                if(!file.is_open())
                {
                    CLAID_THROW(claid::Exception, "Error, cannot save binary data to \"" << path << "\".\n"
                    << "Could not open File for writing.");
                }

                file.write(this->data.data(), this->data.size());
            }
     

        private:

            std::vector<char> data;

            template<typename T>
            static const char* toBinary(const T& value, size_t& size)
            {
                size = sizeof(T);
                return reinterpret_cast<const char*>(&value);
            }

            
            
    };
}



