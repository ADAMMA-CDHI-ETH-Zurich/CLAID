#pragma once
#include <vector>
#include "Reflection/SplitReflect.hpp"

namespace portaible
{
    class BinaryData
    {
        
       
        public:
            // Should only be enabled for primitive types
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
                size_t size = value.size();
                this->store(size);

                // Store characters
                for(size_t i = 0; i < size; i++)
                {
                    this->store(value[i]);
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

            std::vector<char>& getData()
            {
                return this->data;
            }

            size_t getNumBytes()
            {
                return this->data.size();
            }

            template<typename Reflector>
            void reflect(Reflector& r)
            {
                splitReflect(r, *this);
            }

            void clear()
            {
                this->data.clear();
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



namespace portaible
{
    // CAUTION: READ MEANS READ FROM "DATA" (NOT FROM FILE) -> SERIALIZATION
    template<typename Reflector>
    void reflectRead(Reflector& r, BinaryData& binaryData)
    {
        size_t bytes;
        r.member("NumBytes", bytes, "");

        binaryData.resize(bytes);

        char* dataPtr = binaryData.getRawData();
        r.read(dataPtr, bytes);
    }

    // CAUTION: WRITE MEANS WRITE TO "DATA" (NOT TO FILE) -> DESERIALIZATION
    template<typename Reflector>
    void reflectWrite(Reflector& r, BinaryData& binaryData)
    {
        size_t bytes;
        r.member("NumBytes", bytes, "");

        r.write(binaryData.getRawData(), bytes);
    }
}
