#include <vector>

namespace portaible
{
    class BinaryData
    {
        
       
        public:
            // Should only be enabled for primitive types
            template<typename T>
            void store(T& value)
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