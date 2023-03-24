#pragma once

#include "CLAID.hpp"

namespace claid
{
    class CSVSerializer : public Serializer<CSVSerializer>
    {
        private:
            bool headerExists;
            std::string header;
            std::string lastData;

            void addValue(std::string name, std::string stringRepresentation)
            {
                if(header == "")
                {
                    header += name;
                }
                else
                {
                    header += "," + name;
                }

                if(lastData == "")
                {
                    lastData = stringRepresentation;
                }
                else
                {
                    lastData += "," + stringRepresentation;
                }
            }

        public:
            EmptyReflect(CSVSerializer)

            std::string getReflectorName()
            {
                return "CSVSerializer";
            }


            template<typename T>
            void callFloatOrDouble(const char* property, T& member)
            {
                addValue(property, std::to_string(member));
            }   

            // Also includes any variants of signed, unsigned, short, long, long long, ...
            template<typename T>
            void callInt(const char* property, T& member)
            {
                addValue(property, std::to_string(member));
            }

            void callBool(const char* property, bool& member)
            {
                addValue(property, member ? "true" : "false");
            }

            // Why template? Because we can have signed and unsigned char.
            template<typename T>
            void callChar(const char* property, T& member)
            {
                addValue(property, std::string(member));
            }

            template<typename T>
            void callString(const char* property, T& member)
            {
                addValue(property, member);
            }

            template<typename T>
            void callBeginClass(const char* property, T& member)
            {
                
            }

            template<typename T>
            void callEndClass(const char* property, T& member)
            {
                
            }


            template<typename T>
            void callPointer(const char* property, T*& member)
            {
                CLAID_THROW(Exception, "Pointers currently not supported by CSVSerializer");
            }
            
            template<typename T>
            void callSharedPointer(const char* property, T& member)
            {
                CLAID_THROW(Exception, "Shared_ptrs currently not supported by CSVSerializer");
            }

            template<typename T>
            void callEnum(const char* property, T& member)
            {
                int32_t m = static_cast<int32_t>(member);
                this->callInt(property, m);
            }

            void count(const std::string& name, int32_t& count)
            {
                // Do nothing
            }

            void countElements(int32_t& count)
            {
                // Do nothing
            }

            void beginSequence()
            {
                // Do nothing
            }

            void itemIndex(const size_t index)
            {
                // Do nothing
            }

            void endSequence()
            {
                // Do nothing
            }
            
            void write(const char* data, size_t size)
            {

            }


            template<typename T>
            void onInvocationStart(T& obj)
            {
                this->lastData = "";
                this->header = "";
                
            }

  
            void enforceName(std::string& name, int idInSequence = 0)
            {
                // Some serializers, like BinarySerializer, might not store the members name (i.e., property parameters),
                // as it is not necessary to be known in the binary data).
                // For some cases, however, it might be necessary to store such strings in the serialized data nevertheless,
                // as it might be needed for deserialization etc.
                // Thus, this function allows to make sure the string "name" is explicitly stored.
           
            }

            void getDataWriteableToFile(std::vector<char>& data)
            {
                // printf("Get byte rep \n");
                // if(this->root == nullptr)
                // {
                //     data.clear();
                // }
                // else
                // {
                //     std::string str;
                //     this->root->toString(str);
                //     printf("XMLSerializer str %s", str.c_str());

                //     std::copy(str.begin(), str.end(), std::back_inserter(data));
                // }

            }

    };
}