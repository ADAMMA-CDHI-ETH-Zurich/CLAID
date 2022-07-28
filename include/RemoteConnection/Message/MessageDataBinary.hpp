#pragma once
#include "Serialization/Serialization.hpp"
#include "Binary/BinaryData.hpp"
#include "RemoteConnection/Message/MessageData.hpp"

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

            template<typename T>
            void set(T& data)
            {
                BinarySerializer serializer;
                serializer.serialize(data);
                this->binaryData = serializer.binaryData;
            }

            template<typename T>
            void get(T& data)
            {
                BinaryDeserializer deserializer;
                BinaryDataReader reader(&this->binaryData);
                deserializer.deserialize(data, reader);
            }
            
        };
    }
}