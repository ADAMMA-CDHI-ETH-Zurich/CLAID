#pragma once
#include <vector>
#include "Reflection/SplitReflectInType.hpp"
#include "Exception/Exception.hpp"
#include <iostream>
#include <fstream>
#include <mutex>

namespace claid
{
    class BinaryData
    {
        
       
        public:
            BinaryData()
            {
            }

            BinaryData(std::vector<char>& data) : data(data)
            {

            }

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
                int32_t size;
                const char* binaryData = toBinary<T>(value, size);

                this->store(binaryData, size);
            }

            void store(const char* data, int32_t size)
            {
                const char* ptr = data;
                this->data.insert(this->data.end(), data, data + size);
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

            void resize(int32_t size)
            {
                int32_t index = 0;
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

            int32_t getNumBytes() const
            {
                return this->data.size();
            }

            int32_t size() const
            {
                return this->getNumBytes();
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
                int32_t numBytes = file.tellg();
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

            void append(const BinaryData& data)
            {
                const char* ptr = data.getConstRawData();
                this->store(ptr, data.getNumBytes());
            }
     

        private:

            std::vector<char> data;

            template<typename T>
            static const char* toBinary(const T& value, int32_t& size)
            {
                size = sizeof(T);
                return reinterpret_cast<const char*>(&value);
            }           
    };
}

