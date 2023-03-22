#pragma once
#include "Serialization/Serialization.hpp"
#include "Binary/BinaryData.hpp"
#include "RemoteConnection/Message/MessageData/MessageData.hpp"

namespace claid
{
    namespace RemoteConnection
    {
        // Can be used for any data types.
        // Data is set and get by the appropriate functions and de-/serialized automatically.
        // Theoretically, we could always just use this for any data we sent.
        // The other data types (e.g. MessageDataString) are just created for convenience.
        struct MessageDataBinary : public MessageData
        {
            //DECLARE_SERIALIZATION(MessageDataBinary)
            BinaryData binaryData;

            template<typename Reflector>
            void reflect(Reflector& r)
            {
                r.member("Data", binaryData, "");
            }

            void setBinaryData(const BinaryData& binaryData)
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
                BinaryDeserializer deserializer;
                BinaryDataReader reader(&this->binaryData);

                deserializer.deserialize(data, reader); 
            }
            
        };
    }
}