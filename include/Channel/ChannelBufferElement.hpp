#pragma once

#include "ChannelData.hpp"
#include "Binary/BinaryData.hpp"
#include "XML/XMLNode.hpp"
#include "Exception/Exception.hpp"
#include "Serialization/AbstractSerializer.hpp"
#include <mutex>
#include "ClassFactory/ClassFactory.hpp"
#include "Reflection/UntypedReflector.hpp"
#include "Reflection/ReflectionManager.hpp"
#include "Binary/BinaryDeserializer.hpp"
namespace claid
{
   
    class ChannelDataBase;
    template<typename T>
    class ChannelData;

    struct ChannelBufferElement
    {
        // ChannelData is used by typed channels,
        // BinaryData is used by untyped channels.
        // They can be converted to each other, as ChannelData
        // needs to be binary de-/serializable.
        // So, why store both in one struct then, isn't it overhead?
        // Typed channels also allow to read binary data. Whenever
        // binary data shall be read from a channel, the data at the given
        // timestamp needs to be serialized. Thus, if multiple different processes
        // want to read the binary data, it always would need to be serialized again.
        // Thus, whenever the corresponding data of the typed channel is serialized,
        // we store the serialized data aswell.

        protected:
            TaggedDataBase header;
            TaggedData<BinaryData> binaryData;

            // If data is available at all (either binary or typed)
            bool dataAvailable = false;

            // If binary data is available (either set directly or typed data has been serialized).
            bool binaryDataAvailable = false;
            std::mutex mutex;

            std::string dataTypeName;
    

        public:


            ChannelBufferElement(std::string dataTypeName) : dataAvailable(false), dataTypeName(dataTypeName)
            {

            }

            ChannelBufferElement(TaggedData<BinaryData> binaryData, std::string dataTypeName) : binaryData(binaryData), dataAvailable(true), binaryDataAvailable(true), dataTypeName(dataTypeName)
            {
                this->header = binaryData.getHeader();
            }
            
            virtual ~ChannelBufferElement()
            {
            }

            void lock()
            {
                this->mutex.lock();
            }

            void unlock()
            {
                this->mutex.unlock();
            }

            TaggedDataBase getHeader()
            {
                return this->header;
            }

            virtual TaggedData<BinaryData> getBinaryData()
            {
                std::unique_lock<std::mutex> (this->mutex);
                if(!this->dataAvailable)
                {
                    CLAID_THROW(Exception, "Error! Tried to get binary data from ChannelBufferElement (untyped), but no data was ever set (no data available).");
                }

                // This does not copy data, since TaggedData uses a shared_ptr internally.
                TaggedData<BinaryData> data = this->binaryData;

                return data;
            }

            bool isDataAvailable() const
            {
                return this->dataAvailable;
            }

            virtual bool canSerializeToXML() const
            {
                // Cannot serialize to XML, because 
                // we are an untyped ChannelBufferElement.
                return false;
            }

            virtual std::shared_ptr<XMLNode> toXML()
            {
                CLAID_THROW(Exception, "Cannot serialize data to XML.\n"
                << "The channel's buffer (ChannelBuffer) is untyped.\n"
                << "Can only serialize, if there is at least one typed channel instance \n"
                << "available for the targeted channel with a given channel ID in this process.\n"
                << "Make sure there is at least one typed publisher or subscriber for this channel"
                << "in the current instance of CLAID.");
            }

            virtual bool applySerializerToData(std::shared_ptr<AbstractSerializer> serializer)
            {

                // The ChannelBuffer is untyped.
                // Hence, we have to deserialize the data from the binary format first, and apply the serializer afterwards.
                // Therefore, we have to look up the corresponding untyped reflectors, both for the BinaryDeserializer as well as the serializer we shall apply.

                std::string dataTypeName = this->dataTypeName;
                std::string reflectorName = serializer->getReflectorName();

                std::shared_ptr<void> untypedInstance = ClassFactory::getInstance()->getNewUntypedInstance(dataTypeName);
                if(untypedInstance == nullptr)
                {
                    CLAID_THROW(claid::Exception, "Cannot apply serializer to untyped data received on channel.\n"
                    << "The data is of type \"" << dataTypeName << "\", which is unknown to the class factory of this instance of CLAID. This might happen due to one of the following reasons:\n"
                    << "1st The class was not registered to the CLAID serialization system using REGISTER_SERIALIZATION(...). Please make sure that you register the class appropriately.\n"
                    << "2nd The data was received from a remotely connected instance of CLAID, that defined a new data type (e.g., in a separate package), which is not known to this instance of CLAID.\n"
                    << "Make sure that if you want to send data between multiple instances of CLAID, both instances including the packages that contain the corresponding data type.\n"
                    << "If you defined a data type outside of a package, then make sure that this definition is available (at compile time) for both the local and remote instance of CLAID.\n"
                    << "TO SOLVE THIS ISSUE: Either register the class using REGISTER_SERIALIZATION (...) OR let another Module subscribe to the corresponding channel in a typed manner, i.e.: subscribe<" << dataTypeName << ">");
                }

                UntypedReflector* untypedBinaryDeserializer;
                if (!ReflectionManager::getInstance()->getReflectorForClass(dataTypeName, "BinaryDeserializer", untypedBinaryDeserializer))
                {
                    CLAID_THROW(claid::Exception, "Cannot apply serializer to untyped data received on channel.\n"
                    << "The data is of type \"" << dataTypeName << "\", which is unknown to this instance of CLAID. This might happen due to one of the following reasons:\n"
                    << "1st The class was not registered to the CLAID serialization system using REGISTER_SERIALIZATION(...). Please make sure that you register the class appropriately.\n"
                    << "2nd The data was received from a remotely connected instance of CLAID, that defined a new data type (e.g., in a separate package), which is not known to this instance of CLAID.\n"
                    << "Make sure that if you want to send data between multiple instances of CLAID, both instances including the packages that contain the corresponding data type.\n"
                    << "If you defined a data type outside of a package, then make sure that this definition is available (at compile time) for both the local and remote instance of CLAID.");
                }

                std::shared_ptr<BinaryDataReader> reader = std::make_shared<BinaryDataReader>(this->binaryData.value());
                BinaryDeserializer binaryDeserializer(reader);
                
                untypedBinaryDeserializer->invoke(static_cast<void*>(&binaryDeserializer), untypedInstance.get());             
                printf("Apply 6");

                UntypedReflector* untypedSerializer;
                if (!ReflectionManager::getInstance()->getReflectorForClass(dataTypeName, reflectorName, untypedSerializer))
                {
                    return false;
                }
                untypedSerializer->invoke(static_cast<void*>(serializer.get()), untypedInstance.get());               
                return true;
            }

            



    };
}
