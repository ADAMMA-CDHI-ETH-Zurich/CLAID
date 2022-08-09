#pragma once
#include "Serialization/Serialization.hpp"
#include "Binary/BinaryData.hpp"
#include "RemoteConnection/Message/MessageData/MessageData.hpp"

namespace portaible
{
    namespace RemoteConnection
    {
        struct MessageDataBinary : public MessageData
        {
            DECLARE_SERIALIZATION(MessageDataBinary)
            BinaryData binaryData;

            template<typename Reflector>
            void reflect(Reflector& r)
            {
                r.member("Data", binaryData, "");
            }

            void setBinaryData(BinaryData& binaryData)
            {
                this->binaryData = binaryData;
            }

            void getBinaryData(BinaryData& binaryData)
            {
                binaryData = this->binaryData;
            }

            template<typename T>
            void set(T& data)
            {
                BinarySerializer serializer;
                serializer.serialize(data, &binaryData);
               
            }

            template<typename T>
            void get(T& data)
            {
                Logger::printfln("Message data binary des 1 %d", binaryData.getNumBytes());
                BinaryDeserializer deserializer;
                BinaryDataReader reader(&this->binaryData);
                                Logger::printfln("Message data binary des 2");

                deserializer.deserialize(data, reader);
                               Logger::printfln("Message data binary des 3");
 
            }
            
        };
    }
}